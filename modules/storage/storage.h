#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>


class Storage : public QObject
{
    Q_OBJECT
public:
    Storage();
    virtual ~Storage();
    
public://公有函数
    inline void setUser(QString usr,QString p){username=usr;pwd=p;}             //设置用户
    bool upload(const QByteArray &file);                                        //上传文件并返回成功情况
    bool remove();                                                              //删除所有文件
    QByteArray get();                                                           //获取但不删除
    
private://私有对象&变量
    QString username;
    QString pwd;
};