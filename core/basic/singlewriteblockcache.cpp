#include "singlewriteblockcache.h"
#include "../../general.h"
#include "qeventloop.h"
#include <QWaitCondition>
#include <QApplication>


SingleWriteBlockCache::SingleWriteBlockCache():
    state(Invalid)
{
    file = new QTemporaryFile;
    if(!file->open()){
        ncritical<<"FILE OPEN FAILED";
        state=Error;
    }
    thread = new SingleWriteBlockCacheWorkingThread(*this);
    thread->start();
}


SingleWriteBlockCache::~SingleWriteBlockCache(){
    thread->safeStop();
    thread->wait(QDeadlineTimer(5000));
    thread->terminate();
    delete thread;
    delete file;
}

#define REQUIRE_STATE(_state,ret) \
    if(state!=(_state)){ \
        ncritical<<"State check failed:requires"<<#_state<<"Actual"<<state;\
        return ret;\
    }


void SingleWriteBlockCache::clear(){
    if(state==Error){
        return;
    }
    
    thread->safeStop();
    thread->wait();
    state=Invalid;
    QMutexLocker locker(&mutex);
    file->resize(0);
    cache.clear();
    chunkSize=0;
    maxRead=0;
    // file->moveToThread(QThread::currentThread());
    delete file;
    file = new QTemporaryFile;
    file->open();
    delete thread;
    thread=new SingleWriteBlockCacheWorkingThread(*this);
    thread->start();
    currentRead=0;
}


void SingleWriteBlockCache::init(qsizetype chunkSize){
    REQUIRE_STATE(Invalid,);
    this->chunkSize=chunkSize;//atomic无需加锁
    state=Writing;
}


qsizetype SingleWriteBlockCache::append(const char *data){
    REQUIRE_STATE(Writing,-1);
    file->seek(file->size());
    file->write(data,chunkSize);
    return file->size()/chunkSize-1;//返回区块号而非区块数
}


void SingleWriteBlockCache::finalize(){
    REQUIRE_STATE(Writing,);
    state=Reading;
    file->moveToThread(thread);
    maxRead=0;
    thread->wake();
}


QByteArray SingleWriteBlockCache::read(qsizetype idx){
    REQUIRE_STATE(Reading,"");
    QEventLoop loop;
    QObject::connect(thread,&SingleWriteBlockCacheWorkingThread::roundFinished,&loop,[&]{loop.quit();});
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    for(int retry=0;retry<100;retry++){
        //读取
        QByteArray ret;
        bool got = true;
        {
            QMutexLocker locker(&mutex);
            if(cache.contains(idx)){
                ret = cache.value(idx);
            }
            else{//读取
                // locker.unlock();
                got=false;
            }
        }
        //加载到内存
        if(idx>maxRead){
            maxRead=idx;
        }
        currentRead=idx;
        
        thread->wake();
        
        if(got){
            // QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
            return ret;
        }
        else{
            ndb<<"缓存未命中";
            timer.start(10000);
            loop.exec();
            timer.stop();
        }
    }
    ncritical<<"读取失败";
    return "";
}

void SingleWriteBlockCacheWorkingThread::run(){
    while(!isInterruptionRequested()){
        //阻塞等待
        {
            QMutexLocker locker(&mutex);
            while(!needsWake && !isInterruptionRequested()){
                cond.wait(&mutex);
            }
            if (isInterruptionRequested())
            {
                break;
            }
            needsWake=false;
        }
        
        //获取数据
        qsizetype maxRead=c.maxRead,
            chunkSize=c.chunkSize,
            currentRead=c.currentRead,
            begin,end,fileSize;
        bool needsLoad = false;
        {
            QMutexLocker locker(&c.mutex);//持有主类的锁
            if(c.cache.empty()){
                begin=end=0;
            }
            else{
                begin=c.cache.firstKey();
                end=c.cache.lastKey();
            }
            fileSize=c.file->size();
            needsLoad=!c.cache.contains(currentRead);
        }
        //加载文件
        qsizetype aftStart=qMax(maxRead-PRE_CHUNKS,0),
            aftEnd=qMin(maxRead+AFTER_CHUNKS,fileSize/chunkSize-1),
            loadStart=end,
            loadEnd=aftEnd;
        if(loadEnd<loadStart){
            roundFinished();
            continue;
        }
        QMap<qsizetype,QByteArray> map;
        QByteArray currChunk;
        if(needsLoad){
            c.file->seek(chunkSize*currentRead);
            currChunk=c.file->read(chunkSize);
        }
        QByteArray tmp;
        c.file->seek(chunkSize*loadStart);
        tmp=c.file->read((loadEnd-loadStart+1)*chunkSize);
        for(qsizetype i=loadStart;i<=loadEnd;++i){
            map[i]=tmp.mid((i-loadStart)*chunkSize,chunkSize);
        }
        {
            QMutexLocker locker(&c.mutex);
            c.cache.insert(map);
            //淘汰旧值
            c.cache.erase(c.cache.begin(),c.cache.lowerBound(aftStart));
            //淘汰区块兜底
            if(needsLoad){
                c.cache.insert(currentRead,currChunk);
            }
        }
        emit roundFinished();
    }
}

SingleWriteBlockCacheWorkingThread::SingleWriteBlockCacheWorkingThread(SingleWriteBlockCache &c)
    : c(c) {}

void SingleWriteBlockCacheWorkingThread::wake(){
    QMutexLocker locker(&mutex);
    needsWake=true;
    cond.wakeOne();
}

void SingleWriteBlockCacheWorkingThread::safeStop()
{
    requestInterruption();
    QMutexLocker locker(&mutex);
    needsWake=true;
    cond.wakeOne();
}
