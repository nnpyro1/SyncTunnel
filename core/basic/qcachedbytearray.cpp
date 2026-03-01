#include "qcachedbytearray.h"
#include <QFile>
#include <QDataStream>
#include <algorithm>
#include <stdexcept>


QCachedByteArray::QCachedByteArray():tempFile(new QTemporaryFile),mutex_foa(new QMutex)
{
//    tempFile->open();
    if(!tempFile->open()){
        throw std::runtime_error("QCachedByteArray: failed to open temporary file");
    }
    tempFile->setAutoRemove(true);
    is_using_file = true;
}

QCachedByteArray::~QCachedByteArray()
{
    tempFile->close();
}

QCachedByteArray::QCachedByteArray(QByteArray ba):tempFile(new QTemporaryFile),mutex_foa(new QMutex)
{
//    tempFile->open();
    if(!tempFile->open()){
        throw std::runtime_error("QCachedByteArray: failed to open temporary file");
    }
    tempFile->setAutoRemove(true);
    is_using_file = true;
    tempFile->write(ba);
}

char QCachedByteArray::at(qint64 i) const{
    QMutexLocker locker(mutex_foa.data());
    if(i<0||i>=size()){
        throw std::out_of_range("QCachedByteArray::at(long i) i out of range");
    }
    if(is_using_file){
        if(!tempFile->seek(i)){
            throw std::runtime_error("QCahcedByteArray::at(long i) unable to seek");
        }
        char byte;
        if(tempFile->read(&byte,1)!=1){
            throw std::runtime_error("QCachedByteArray::at(long i) unable to read");
        }
        return byte;
    }
    else{
        if(i > INT_MAX){
            throw std::out_of_range("QCachedByteArray::at(long i) out of range [0,INT_MAX]");   
        }
        else{
            return array[(int)i];
        }
    }
}

qint64 QCachedByteArray::size() const{
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        return tempFile->size();
    }
    else{
        return array.size();
    }
}

void QCachedByteArray::append(QByteArray ar){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        if(!tempFile->seek(tempFile->size())){
            throw std::runtime_error("QCahcedByteArray::append(QByteArray ar) unable to seek");
        }
        tempFile->write(ar);
    }
    else{
        array.append(ar);
    }
}

void QCachedByteArray::set(qint64 pos, QByteArray byte){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        if(!tempFile->seek(pos)){
            throw std::runtime_error("QCachedByteArray::set(long pos, QByteArray byte) unable to seek");
        }
        tempFile->write(byte);
    }
    else{
        char* dest = array.data() + pos;
        const char* src = byte.constData();
        memcpy(dest, src, byte.size());
    }
}

QByteArray QCachedByteArray::mid(qint64 pos, qsizetype len){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        if(!tempFile->seek(pos)){
            throw std::runtime_error("QCachedByteArray::mid(long pos, int len) unable to seek");
        }
        QByteArray ret;
        ret.resize(len);
        if(tempFile->read(ret.data(),len)!=len){
            throw std::runtime_error("QCachedByteArray::mid(long pos, int len) unable to read");
        }
        return ret;
    }
    else{
        return array.mid(pos,len);
    }
}

QCachedByteArray QCachedByteArray::mid_cached(qint64 pos, qsizetype len){
    return QCachedByteArray(this->mid(pos,len));
}


QByteArray QCachedByteArray::data(){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        tempFile->seek(0);
        return tempFile->readAll();
    }
    else{
        return array;
    }
}

void QCachedByteArray::load(){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        tempFile->seek(0);
        array = tempFile->readAll();
        is_using_file = false;
    }
    else{
        
    }
}

void QCachedByteArray::unload(){
    QMutexLocker locker(mutex_foa.data());
    if(is_using_file){
        
    }
    else{
        tempFile->seek(0);
        tempFile->resize(array.size());
        tempFile->write(array);
        array.clear();
        is_using_file = true;
    }
}


/*void QCachedByteArray::fullLoad()
{
    
}

void QCachedByteArray::storeToFile()
{
    
}*/