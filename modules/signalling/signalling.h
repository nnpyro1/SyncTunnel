#ifndef SIGNALLING_H
#define SIGNALLING_H

#include "qendian.h"
#include <QObject>
#include <general.h>
#include <QMqttClient>

class Signalling : public QObject
{
    Q_OBJECT
public:
    explicit Signalling(QObject *parent = nullptr);
    ~Signalling();
    void setPassport(QString username,QString pwd);
    void setPublicIp(device public_ip);
    void setMqttBroker(QString host,quint16 port);
    
    bool start();
    void stop();
    
    void registerOnline();
    Devices getAllDevices();
    void registerOffline();
    
    bool isAvailable();
    
signals:
    void deviceOnline(devid_t d);
    void deviceOffline(devid_t d);
    void deviceUpdated();
    
private slots:
    void mqttReadyRead(QByteArray msg);
    
private:
    enum PackageType{
        RegisterOnline,
        DeviceInfo,
        RegisterOffline,
    };
#pragma pack(push,1)
    // struct BasicPackage{
    //     qint32 type;
    //     char ip[46];
    //     quint16 port;
    // };
    struct BasicPackage{
        qint32 type;
        char ip[46];
        quint16 port;
        qint32 flag;
        //剩下载荷为description
    };
#pragma pack(pop)
    
    QByteArray encode(QByteArray data);
    QByteArray decode(QByteArray data);
    
private:
    QString username;
    QString password;
    device public_ip;
    Devices clients;
    QString mqttHost;
    quint16 mqttPort;
    QMqttClient *client;
    QMqttSubscription *subscription;
    QTimer finishTimer;
};

#endif // SIGNALLING_H
