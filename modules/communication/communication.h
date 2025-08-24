#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDataStream>


class Communication : public QObject
{
    Q_OBJECT
    
public://公有声明
    Communication();
    virtual ~Communication();
    
    struct ipport{
        QString ip;
        quint16 port;
        
        inline operator const QString(){
            return QString("%1:%2").arg(ip).arg(port);
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
    };
    
signals:
    void readyRead();
    
public://函数
    ipport stun();                                                      //发起STUN请求并返回公网IP端口号
    ipport getIPv6();                                                   //获取公网IPv6地址。此函数是对称NAT下STUN的替代，调用后会覆盖stun()的结果。
//    bool bind(int port);                                              //绑定对应端口                  //废弃接口
    qint64 send(ipport host,QByteArray msg);                            //发送消息
    QNetworkDatagram readDatagram();                                    //读取数据包
    inline void setStunServer(ipport server){stun_host=server;}         //设置服务器
    
private://私有函数
    void on_read(){emit readyRead();}
    
private://私有变量&对象
    QUdpSocket *socket;
    QUdpSocket *socket_stun;
    QUdpSocket *socket_ipv6;
    ipport stun_host = {"stun.miwifi.com",3478}/*{"stun.l.google.com",19302}*/;
};