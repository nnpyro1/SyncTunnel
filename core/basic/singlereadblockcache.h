#ifndef SINGLEREADBLOCKCACHE_H
#define SINGLEREADBLOCKCACHE_H

#include "filebytearray.h"
#include "qbitarray.h"
#include "qmutex.h"
#include "qthread.h"
#include "qwaitcondition.h"
#include <QHash>

class SingleReadBlockCacheWorkingThread;
class SingleReadBlockCache
{
    friend class SingleReadBlockCacheWorkingThread;
public:
    SingleReadBlockCache();
    ~SingleReadBlockCache();
    
    void init(qsizetype chunkSize,qsizetype totalChunkNum);             ///< 初始化指定块大小
    bool write(qsizetype chunkId,const QByteArray &data);               ///< 向指定块写入数据。存在将会覆盖
    bool hasValue(qsizetype chunkId);                                   ///< 返回chunkId指向的块是否曾经写入过
    qsizetype size();
    
    /// 读取所有值 
    /// @warning 此函数将会重置所有数据，只允许使用一次
    FileByteArray takeAwayWholeData();
    
private:
    QTemporaryFile *f=nullptr;;
    SingleReadBlockCacheWorkingThread *thread=nullptr;
    QHash<qsizetype,QByteArray> buf;
    QMutex bufMutex;
    QBitArray written;
    
    // bool needsTakeAway = false;
    // QMutex needsTakeAwayMutex;
    QAtomicInteger<bool> needsTakeaway = false;
    QAtomicInteger<bool> needsStop = false;
    bool valid=false;
    
    QWaitCondition cond;
    
    QAtomicInteger<qsizetype> chunkSize,totalChunkNum;
    
    QThread *currentThread;
};


class SingleReadBlockCacheWorkingThread : public QThread
{
    Q_OBJECT
public:
    SingleReadBlockCacheWorkingThread(SingleReadBlockCache &c):c(c){}
    void safeStop();
signals:
    void fin();
protected:
    void run() override;
private:
    SingleReadBlockCache &c;
};

#endif // SINGLEREADBLOCKCACHE_H
