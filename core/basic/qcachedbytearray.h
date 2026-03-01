#ifndef QCACHEDBYTEARRAY_H
#define QCACHEDBYTEARRAY_H

#include <QByteArray>
#include <QTemporaryFile>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>


class QCachedByteArray {
    
public:
    QCachedByteArray();
    ~QCachedByteArray();
    QCachedByteArray(QByteArray ba);
    
    //接管函数
    char at(qint64 i) const;
    qint64 size() const;
    void append(QByteArray ar);
    void set(qint64 pos,QByteArray byte);
    QByteArray mid(qint64 pos,qsizetype len);
    QCachedByteArray mid_cached(qint64 pos,qsizetype len);
    
    //自定义函数
//    void fullLoad();
//    void storeToFile();
    QByteArray data();
    void load();
    void unload();
    
private://私有函数
    
private://私有变量/对象
    mutable QSharedPointer<QTemporaryFile> tempFile;
    mutable QByteArray array;
    bool is_using_file = true;
    mutable QSharedPointer<QMutex> mutex_foa;
};

#endif // QCACHEDBYTEARRAY_H
