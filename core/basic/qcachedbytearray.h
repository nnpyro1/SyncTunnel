/************************************************************************
 * 文件：qcachedbytearray.h
 * 作者：nnyjnte代
 * 
 * 本文件及其相关实现采用"无限制授权"：
 * 1. 任何个人或组织均可自由使用、复制、修改、合并本作品
 * 2. 允许用于任何目的（包括商业用途），无需授权
 * 3. 不提供任何担保，使用者承担所有风险
 ************************************************************************/
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
    char at(long i) const;
    long size() const;
    void append(QByteArray ar);
    void set(long pos,QByteArray byte);
    QByteArray mid(long pos,int len);
    QCachedByteArray mid_cached(long pos,int len);
    
    //自定义函数
//    void fullLoad();
//    void storeToFile();
    QByteArray data();
    
    
private://私有函数
    
private://私有变量/对象
    mutable QSharedPointer<QTemporaryFile> tempFile;
    mutable QByteArray array;
    bool is_using_file = true;
    mutable QSharedPointer<QMutex> mutex_foa;
};

#endif // QCACHEDBYTEARRAY_H
