#ifndef FILEBYTEARRAY_H
#define FILEBYTEARRAY_H


#include "qtypes.h"
#include <QByteArray>
#include <QTemporaryFile>


class FileByteArray
{
public:
    FileByteArray();
    FileByteArray(const FileByteArray &) = delete;
    FileByteArray(FileByteArray &&) noexcept;
    explicit FileByteArray(QTemporaryFile *f) noexcept;
    FileByteArray &operator=(const FileByteArray &) noexcept = delete;
    FileByteArray &operator=(FileByteArray &&) noexcept;
    ~FileByteArray();
    
    //接管QByteArray
    qsizetype size() const;                             ///< 获取总大小
    bool isEmpty() const;                               ///< 是否空
    void clear();                                       ///< 清空
    char at(qsizetype i) const;                         ///< 读取i索引处的字节   @warning 注意此函数只读无法写！！！
    char operator[](qsizetype i) const;                 ///< 同at()            @warning 注意此函数只读无法写！！！
    QByteArray mid(qsizetype pos,qsizetype len=-1) const;///< 在pos处取len个字节
    QByteArray left(qsizetype len) const;               ///< 取开头len个字节
    QByteArray right(qsizetype len) const;              ///< 取结尾len个字节
    void append(const QByteArray &data);                ///< 尾部追加data
    void append(const char ch);                         ///< 尾部追加一个ch字节
    void append(const char *ch,qsizetype len);          ///< 尾部追加ch开始的len个字节
    
    //自定义API
    void writeAt(qsizetype pos,const QByteArray &ba);   ///< 在pos处写入并覆盖后续内容（若有）
    void writeAt(qsizetype pos,char ch);                ///< 在pos处写入并覆盖后续内容（若有）
    void writeAt(qsizetype pos,qsizetype len,const char*);
    QTemporaryFile *file();                             ///< 获取原始文件 @warning 仅可进行读写操作，禁止close
    QByteArray readAll() const;                         ///< 读取所有 @warning 此函数慎用！！！
    
    
private:
    QTemporaryFile *f;
};

#endif // FILEBYTEARRAY_H
