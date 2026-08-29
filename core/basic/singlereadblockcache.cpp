#include "singlereadblockcache.h"
#include "general.h"
#include "qeventloop.h"
#include <QMutexLocker>

SingleReadBlockCache::SingleReadBlockCache(){
    currentThread=QThread::currentThread();
}

SingleReadBlockCache::~SingleReadBlockCache(){
    ndb<<"析构函数";
    if(thread){
        ndb<<"析构2";
        
        //直接停止线程
        thread->safeStop();
        ndb<<"析构3";
        ndb<<"停止前Thread正在运行："<<thread->isRunning();
        thread->quit();
        ndb<<"析构4";
        ndb<<"停止成功:"<<thread->wait(50000);
        ndb<<"析构5";
        // if(!thread->isFinished()){
        //     ndb<<"析构5‑1";
        //     // thread->terminate();
        //     ndb<<"析构5‑2";
        // }
        
        //迁移f到当前线程再销毁
        if(f){
            ndb<<"f属于当前线程："<<(f->thread()==QThread::currentThread());
            f->moveToThread(QThread::currentThread());
            delete f;
            f=nullptr;
            ndb<<"析构8";
        }
        
        ndb<<"析构9";
        delete thread;
        thread=nullptr;
        ndb<<"析构10";
    }
}

void SingleReadBlockCache::init(qsizetype chunkSize,qsizetype totalChunkNum){
    valid=true;
    thread = new SingleReadBlockCacheWorkingThread(*this);
    f = new QTemporaryFile;
    if(!f->open()){
        ncritical<<"Unable to open file";
        valid=false;
        delete f;
        delete thread;
        f=nullptr;
        thread=nullptr;
        return;
    }
    f->moveToThread(thread);
    thread->start();
    
    this->chunkSize=chunkSize;
    this->totalChunkNum=totalChunkNum;
    
    {
        QMutexLocker locker(&bufMutex);
        buf.reserve(500);
    }
    written = QBitArray(totalChunkNum,false);
}

bool SingleReadBlockCache::write(qsizetype chunkId, const QByteArray &data){
    if(!valid){
        ncritical<<"SingleReadBlockCache not valid.";
        return false;
    }
    {
        QMutexLocker locker(&bufMutex);
        buf.insert(chunkId,data);
    }
    written.setBit(chunkId,true);
    cond.wakeOne();
    return true;
}

bool SingleReadBlockCache::hasValue(qsizetype chunkId){
    if(!valid){
        ncritical<<"SingleReadBlockCache not valid.";
        return false;
    }
    return written.testBit(chunkId);
}

qsizetype SingleReadBlockCache::size(){
    return totalChunkNum;
}

FileByteArray SingleReadBlockCache::takeAwayWholeData(){
    if(!valid){
        ncritical<<"SingleReadBlockCache not valid.";
        return {};
    }
    
    // 通知子线程：刷完剩余缓冲区，然后退出
    {
        QMutexLocker locker(&bufMutex);
        needsTakeaway = true;
    }
    cond.wakeOne();
    
    // 停止并等待子线程完全结束
    thread->safeStop();
    thread->quit();
    thread->wait(50000);
    // if(!thread->isFinished()){
    //     thread->terminate();
    // }
    
    // 子线程已退出，迁移 QTemporaryFile 到主线程
    f->moveToThread(QThread::currentThread());
    
    // 清空内部状态
    {
        QMutexLocker locker(&bufMutex);
        buf.clear();
    }
    written.clear();
    needsTakeaway=needsStop=valid=false;
    chunkSize=totalChunkNum=0;
    
    delete thread;
    thread=nullptr;
    
    auto tmp=f;
    f=nullptr;
    return FileByteArray(tmp);
}

void SingleReadBlockCacheWorkingThread::safeStop(){
    QMutexLocker locker(&c.bufMutex);
    c.needsStop = true;
    c.cond.wakeAll();
}

void SingleReadBlockCacheWorkingThread::run(){
    forever{
        {
            QMutexLocker locker(&c.bufMutex);
            while(!c.needsStop && !c.needsTakeaway && c.buf.empty()){
                c.cond.wait(&c.bufMutex);
            }
        }
        
        if(c.needsStop){
            c.f->moveToThread(c.currentThread);
            return;
        }
        
        //刷盘
        QList<QPair<qsizetype,QByteArray>> data;
        {
            QMutexLocker locker(&c.bufMutex);
            if(!c.buf.empty()){
                data.resize(c.buf.size());
                auto d = data.data();
                for(auto it=c.buf.begin();it!=c.buf.end();++it){
                    *(d++)=qMakePair((it.key()),std::move(it.value()));
                }
                c.buf.clear();
            }
        }
        
        for(const auto &i:std::as_const(data)){
            if(c.f->seek(c.chunkSize*i.first)&&c.f->write(i.second)>=0){
                //ok
            }
            else{
                ncritical<<"Cannot write data.";
            }
        }
        
        //退出标记
        {
            QMutexLocker locker(&c.bufMutex);
            if(c.needsTakeaway){
                c.f->moveToThread(c.currentThread);
                return;
            }
        }
    }
}
