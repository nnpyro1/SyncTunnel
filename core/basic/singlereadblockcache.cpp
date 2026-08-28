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
        if(f){
            ndb<<"析构2-1";
            needsTakeaway = true;
            ndb<<"析构2-2";
            //等待文件移交
            QEventLoop loop;
            QObject::connect(thread,&SingleReadBlockCacheWorkingThread::fin,&loop,&QEventLoop::quit);
            ndb<<"析构2-3";
            cond.wakeOne();
            ndb<<"析构2-4";
            loop.exec();
            ndb<<"析构2-5";
        }
        ndb<<"析构3";
        ndb<<"停止前Thread正在运行："<<thread->isRunning();
        thread->safeStop();
        ndb<<"析构4";
        thread->quit();
        ndb<<"析构5";
        ndb<<"停止成功:"<<thread->wait(5000);
        ndb<<"析构6";
        if(!thread->isFinished()){
            ndb<<"析构6-1";
            thread->terminate();
            ndb<<"析构6-2";
        }
        ndb<<"析构7";
        ndb<<"f属于当前线程："<<(f->thread()==QThread::currentThread());
        delete f;
        ndb<<"析构8";
        f=nullptr;
        ndb<<"析构9";
        delete thread;
        ndb<<"析构10";
        thread=nullptr;
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
    needsTakeaway = true;
    //等待文件移交
    QEventLoop loop;
    QObject::connect(thread,&SingleReadBlockCacheWorkingThread::fin,&loop,&QEventLoop::quit);
    cond.wakeOne();
    loop.exec();
    //清空状态
    {
        QMutexLocker locker(&bufMutex);
        buf.clear();
    }
    written.clear();
    needsTakeaway=needsStop=valid=false;
    chunkSize=totalChunkNum=0;
    thread->safeStop();
    thread->quit();
    thread->wait(5000);
    if(!thread->isFinished()){
        thread->terminate();
    }
    delete thread;thread=nullptr;
    auto tmp=f;f=nullptr;
    return (FileByteArray(tmp));
}


void SingleReadBlockCacheWorkingThread::safeStop(){
    // QMutexLocker locker(&c.bufMutex);
    c.needsStop = true;
    c.cond.wakeAll();
}


void SingleReadBlockCacheWorkingThread::run(){
    forever{
        //等待
        {
            QMutexLocker locker(&c.bufMutex);
            while(!c.needsStop && !c.needsTakeaway && c.buf.empty()){
                c.cond.wait(&c.bufMutex);
            }
        }
        
        if(c.needsStop){
            return;
        }
        //刷盘
        // bool empty=true;
        {
            QList<QPair<qsizetype,QByteArray>> data;
            {
                QMutexLocker locker(&c.bufMutex);
                if(!c.buf.empty()){
                    // empty=false;
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
                    //成功
                }
                else{
                    ncritical<<"Cannot write data.";
                }
            }
        }
        if(c.needsTakeaway){
            bool empty = false;
            {
                QMutexLocker locker(&c.bufMutex);
                empty=c.buf.empty();
            }
            if(empty){//只有在缓冲区清空后才能释放
                //移交所有权
                c.f->moveToThread(c.currentThread);
                emit fin();
                c.needsStop=true;
                return;
            }
        }
    }
}
