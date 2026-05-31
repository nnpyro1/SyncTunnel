#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QObject>
#include <QDir>
#include <QList>
#include <QPair>
#include <QString>
#include <QMap>
#include <QSet>
#include <core/services/schedule.h>


class Utils
{
public:
    Utils() = delete;//纯静态类，禁止构造
    
public://以下是纯静态函数
    static QByteArray encode(const QByteArray &msg,const QString &pwd);     //加密msg并返回密文
    static QByteArray decode(const QByteArray &msg,const QString &pwd);     //解密msg并返回解密后的值
    static QByteArray mergeFile(QDir folder,QSet<QString> incremental_sync_set=QSet<QString>(),bool c=1); //合并文件
    static void releaseFile(QByteArray msg);                                //释放文件
    static void multiDelay(float ms);                                       //高精度定时
    static QByteArray readLine(QByteArray &ba);                             //读取一行并删除这一行，返回不带换行符的消息
    static QMap<QString,QByteArray> generateFileHashMap(QDir baseDir);      //更新文件哈希表
    static QList<QPair<QFileInfo,QDir>>traverseFolder(QDir folder);         //遍历  
    static QByteArray getMachineUniqueId();                                 //获取设备唯一ID的哈希
    static void restart();                                                  //软重启应用程序
    QByteArray serSchedule(QList<Schedule*> schedule_list);                 //序列号Schedule
//    QList<Schedule *> unserSchedule(QByteArray dat);                        //反序列化schedule
};

#endif // UTILS_H
