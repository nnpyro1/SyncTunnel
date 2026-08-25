#ifndef SINGLEWRITEBLOCKCACHE_H
#define SINGLEWRITEBLOCKCACHE_H


#include "filebytearray.h"
#include "qtypes.h"
#include "qwaitcondition.h"
#include <QThread>
#include <QMap>
#include <QMutex>

class SingleWriteBlockCacheWorkingThread;

class SingleWriteBlockCache
{
    friend class SingleWriteBlockCacheWorkingThread;
public:
    SingleWriteBlockCache();
    ~SingleWriteBlockCache();
    
    enum State{
        Error,
        Invalid,
        Writing,
        Reading,
    };
    
    //重置
    void clear();                       //重置
    
    //写阶段
    void init(qsizetype chunkSize);     //初始化尺寸为chunkSize
    qsizetype append(const char *data); //写入chunksize大小的数据 返回区块号
    void finalize();                    //结束写
    inline State getState() const{return state;}
    
    //读阶段
    QByteArray read(qsizetype idx);     //读取idx处的区块
    
private:
    State state;
    QTemporaryFile *file;
    QMap<qsizetype,QByteArray> cache;
    QMutex mutex;//掌管cache
    SingleWriteBlockCacheWorkingThread *thread;
    QAtomicInteger<qsizetype> chunkSize;
    QAtomicInteger<qsizetype> maxRead;
    QAtomicInteger<qsizetype> currentRead;
    
};

class SingleWriteBlockCacheWorkingThread : public QThread{
    Q_OBJECT
protected:
    void run() override;
private:
    SingleWriteBlockCache &c;
    QMutex mutex;
    QWaitCondition cond;
    bool needsWake=false;
    
    static constexpr const qsizetype PRE_CHUNKS = 200;
    static constexpr const qsizetype AFTER_CHUNKS = 1000;
    
public:
    explicit SingleWriteBlockCacheWorkingThread(SingleWriteBlockCache &c);
    
    void wake();
    void safeStop();
    
signals:
    void roundFinished();
};

#endif // SINGLEWRITEBLOCKCACHE_H
