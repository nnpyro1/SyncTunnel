#include "filebytearray.h"
#include <general.h>


FileByteArray::FileByteArray() {
    f = new QTemporaryFile;
    if(!f->open()){
        ncritical<<"Cannot create temporary file.";
    }
}


FileByteArray::FileByteArray(FileByteArray &&rhs) noexcept{
    f=rhs.f;
    rhs.f=nullptr;
}


FileByteArray::~FileByteArray(){
    delete f;
}

FileByteArray &FileByteArray::operator=(FileByteArray &&rhs) noexcept{
    if(&rhs==this)return *this;
    if(this->f)delete this->f;
    f=rhs.f;
    rhs.f=nullptr;
    return *this;
}

#define CHECK(failed_ret) if(!(f&&f->isOpen())){ncritical<<"Invalid Object";return failed_ret;}

qsizetype FileByteArray::size() const{
    CHECK(0)
    return f->size();
}


bool FileByteArray::isEmpty() const{
    return size()==0;//size有校验
}


void FileByteArray::clear(){
    CHECK()
    f->resize(0);
}


char FileByteArray::at(qsizetype i) const{
    CHECK(0)
    if(!f->seek(i)){
        return 0;
    }
    // return f->read(1).at(0);
    auto ret=f->read(1);    
    return ret.size()>=1?ret.at(0):'\0';
}


char FileByteArray::operator[](qsizetype i) const{
    return at(i);
}


QByteArray FileByteArray::mid(qsizetype pos, qsizetype len) const{
    CHECK({})
    if(pos<0||pos>=size()){
        return {};
    }
    if(!f->seek(pos)){
        return {};
    }
    if(len==-1){//读取全部
        return f->readAll();
    }
    else{
        if(pos+len>size()){
            // return {};
            return f->readAll();
        }
        return f->read(len);
    }
}


QByteArray FileByteArray::left(qsizetype len) const{
    return mid(0,len);
}


QByteArray FileByteArray::right(qsizetype len) const{
    return mid(size()-len);
}


void FileByteArray::append(const QByteArray &data){
    append(data.constData(),data.size());
}


void FileByteArray::append(const char ch){
    append(&ch,1);
}


void FileByteArray::append(const char *ch, qsizetype len){
    CHECK();
    if(f->seek(size()))
        f->write(ch,len);
}


void FileByteArray::writeAt(qsizetype pos, const QByteArray &ba){
    CHECK();
    if(f->seek(pos)){
        f->write(ba);
    }
}


void FileByteArray::writeAt(qsizetype pos, char ch){
    writeAt(pos,1,&ch);
}


void FileByteArray::writeAt(qsizetype pos, qsizetype len, const char *c){
    CHECK();
    if(f->seek(pos)){
        f->write(c,len);
    }
}


QTemporaryFile *FileByteArray::file(){
    return f;
}


QByteArray FileByteArray::readAll() const{
    return mid(0);
}




