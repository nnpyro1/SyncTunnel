#pragma once
#include "qobject.h"
#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDataStream>
#include <QHashFunctions>
#include <QtDebug>
#include <QQueue>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>


class Communication : public QObject
{
    Q_OBJECT
    
public://公有声明
    Communication();
    virtual ~Communication();
    
    struct ipport{
        QString ip;
        quint16 port;
        
        inline QString toString() const{
            if(ip.contains(':')) return QString("[%1]:%2").arg(ip).arg(port);
            else return QString("%1:%2").arg(ip).arg(port);
        }
        inline operator const QString() const{
            return toString();
        }
        inline friend QDataStream& operator<<(QDataStream& out, const ipport& value) {
            out << value.ip << value.port;
            return out;
        }
        inline friend QDataStream& operator>>(QDataStream& in, ipport& value) {
            in >> value.ip >> value.port;
            return in;
        }
        inline friend bool operator==(const ipport &l,const ipport &r){
            return l.ip==r.ip&&l.port==r.port;
        }
        inline bool operator<(const ipport &a)const{
            return ip<a.ip;
        }
    };
    struct device:public ipport{
        QString description="";
        int flag=0;
        device(){}
        device(const ipport p):ipport(p){}
        device(QString ip,quint16 port,QString description="",int flag=0):ipport({ip,port}),description(description),flag(flag){}
        inline friend bool operator==(const device &l,const device &r){
            return l.ip==r.ip&&l.port==r.port;//只比较ip和port
        }
        inline operator const QString() const{
            if(ip.contains(':')) return QString("[%1]:%2").arg(ip).arg(port);
            else return QString("%1:%2").arg(ip).arg(port);
        }
        inline QString toString(){return operator const QString();}
        inline QString toFullString(){return toString()+"("+description+","+QString::number(flag,16)+")";}
        inline friend bool operator==(const device &l,const ipport &r){
            return l.ip==r.ip&&l.port==r.port;
        }
        inline friend bool operator==(const ipport &l,const device &r){return r==l;}
    };
    
    
    enum DeviceFlag{
        WindowsDevice,
        AndroidDevice,
        LinuxDevice,
        
        DFHNDevice = 32,
    };
    Q_ENUM(DeviceFlag)
    
signals:
    void readyRead();
    
public://函数
    ipport stun();                                                      //发起STUN请求并返回公网IP端口号
    ipport getIPv6();                                                   //获取公网IPv6地址。此函数是对称NAT下STUN的替代，调用后会覆盖stun()的结果。
//    bool bind(int port);                                              //绑定对应端口                  //废弃接口
    qint64 send(ipport host,QByteArray msg);                            //发送消息
    QNetworkDatagram readDatagram();                                    //读取数据包
    inline void setStunServer(ipport server){stun_host=server;}         //设置服务器
    bool hasPendingDatagrams();                                         //是否有等待中数据包
    
private://私有函数
    void on_read();
    
private://私有变量&对象
    QUdpSocket *socket;
    QUdpSocket *socket_stun;
    QUdpSocket *socket_ipv6;
    ipport stun_host = /*{"stun.miwifi.com",3478}*//*{"stun.l.google.com",19302}*/{"stun.chat.bilibili.com",3478};
    QQueue<QNetworkDatagram> buf;
    QTimer timer_read;
    
};

inline uint qHash(const Communication::ipport &key, uint seed) noexcept{
    return qHash(key.ip, seed) ^ qHash(key.port, seed);
}

inline QDebug operator<<(QDebug d,const Communication::ipport &i){return d<< i.operator const QString();}
using devid_t = qint32;
typedef QMap<devid_t,Communication::device> Devices;
Q_DECLARE_METATYPE(Communication::device)
Q_DECLARE_METATYPE(Devices)
inline devid_t getIdByDevice(Communication::device dev){
    return (qHash(dev.ip+"@"+QString::number(dev.port))&0x7FFFFFFF);
}
inline QString getStringByDeviceId(devid_t devId){
    QByteArray di('\0',sizeof(devId));
    memcpy(di.data(),&devId,sizeof(devId));
    return QString(di.toBase64(QByteArray::OmitTrailingEquals));
}
inline devid_t getIdByString(QString str){
    auto d = QByteArray::fromBase64(str.toUtf8());
    if(d.size()!=sizeof(devid_t)){
        return 0;
    }
    devid_t ret;
    memcpy(&ret,d.constData(),sizeof(ret));
    return ret;
}