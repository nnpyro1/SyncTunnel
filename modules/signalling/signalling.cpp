#include "signalling.h"
#include "qeventloop.h"
#include <QCryptographicHash>
#include <utils.h>


Signalling::Signalling(QObject *parent)
    : QObject{parent}
{
    
}

Signalling::~Signalling(){
    
}


void Signalling::setPassport(QString username, QString pwd){
    this->username=username;
    this->password=pwd;
}


void Signalling::setPublicIp(device public_ip){
    this->public_ip=public_ip;
}


void Signalling::setMqttBroker(QString host, quint16 port){
    this->mqttHost=host;
    this->mqttPort=port;
}


bool Signalling::start(){
    client = new QMqttClient(this);
    client->setHostname(mqttHost);
    client->setPort(mqttPort);
    
    client->connectToHost();
    QEventLoop loop;//等待到链接完成
    connect(client,&QMqttClient::connected,&loop,&QEventLoop::quit);
    connect(client,&QMqttClient::errorChanged,&loop,&QEventLoop::quit);
    loop.exec();
    if(client->error()!=QMqttClient::NoError){
        ncritical<<"Failed to connect."<<client->error();
        return false;
    }
    connect(client,&QMqttClient::messageReceived,this,&Signalling::mqttReadyRead);
    
    subscription = client->subscribe(QString("synctunnel-signalling/v2/%1").arg(QCryptographicHash::hash((username+"@"+password).toUtf8(),QCryptographicHash::Sha256).toHex()),2);
    return subscription!=0;
}


void Signalling::stop(){
    if(isAvailable())registerOffline();
    if(subscription)subscription->deleteLater();subscription=0;
    if(client)client->deleteLater();client=0;
}


/*
协议流程
A               其他人
RegisterOnline ->
    <- DeviceInfo

RegisterOffline ->
*/


void Signalling::registerOnline(){
    if(!subscription){
        ncritical<<"Invalid subscription";
        return;
    }
    //发送registerOnline
    BasicPackage pk;
    memset(&pk,0,sizeof(pk));
    strcpy(pk.ip,public_ip.ip.toStdString().c_str());
    pk.port=qToBigEndian(public_ip.port);
    pk.type=qToBigEndian((int)RegisterOnline);
    pk.flag=qToBigEndian(public_ip.flag);
    QByteArray msg;
    msg.resize(sizeof(pk));
    memcpy(msg.data(),&pk,sizeof(pk));
    msg.append(public_ip.description.toUtf8());
    
    //发送并阻塞直到结果超时
    ninfo<<"Registering...";
    client->publish(subscription->topic().filter(),encode(msg),2);
    QEventLoop loop;
    connect(&finishTimer,&QTimer::timeout,&loop,&QEventLoop::quit);
    finishTimer.start(5000);
    loop.exec();
    
    //添加自己
    clients.insert(getIdByDevice(public_ip),public_ip);
}


Devices Signalling::getAllDevices(){
    return clients;
}


void Signalling::registerOffline(){
    if(!subscription){
        ncritical<<"Invalid subscription";
        return;
    }
    //发送registerOffline
    BasicPackage pk;
    memset(&pk,0,sizeof(pk));
    strcpy(pk.ip,public_ip.ip.toStdString().c_str());
    pk.port=qToBigEndian(public_ip.port);
    pk.type=qToBigEndian((int)RegisterOffline);
    QByteArray msg;
    msg.resize(sizeof(pk));
    memcpy(msg.data(),&pk,sizeof(pk));
    
    //发送
    qint32 msgid = client->publish(subscription->topic().filter(),encode(msg),2);
    ninfo<<"Registering offline.id="<<msgid;
    // QThread::msleep(500);
    QEventLoop loop;
    connect(client,&QMqttClient::messageSent,this,[&](qint32 id){
        if(id==msgid){loop.quit();}
    });
    QTimer::singleShot(5000,&loop,&QEventLoop::quit);
    loop.exec();
    ninfo<<"offline.";
}


bool Signalling::isAvailable(){
    return client&&subscription;
}


void Signalling::mqttReadyRead(QByteArray msg){
    msg=decode(msg);
    if(msg.size()<(qsizetype)sizeof(BasicPackage)){//包括解密失败的情况
        ncritical<<"Invalid MQTT Payload:"<<msg;
        return;
    }
    BasicPackage bp;
    memcpy(&bp,msg.constData(),sizeof(bp));
    bp.port=qFromBigEndian(bp.port);
    bp.type=qFromBigEndian(bp.type);
    bp.flag=qFromBigEndian(bp.flag);
    QString description = msg.mid(sizeof(bp));;
    
    //忽略自己
    if(bp.ip==public_ip.ip&&bp.port==public_ip.port){
        ninfo<<"Ignore self";
        return;
    }
    
    //解析消息
    if(bp.type == RegisterOnline){//注册上线，回复DeviceInfo
        BasicPackage response;
        memset(&response,0,sizeof(response));
        response.type=qToBigEndian((int)DeviceInfo);
        response.flag=qToBigEndian(public_ip.flag);
        strcpy(response.ip,public_ip.ip.toStdString().c_str());
        response.port=qToBigEndian(public_ip.port);
        QByteArray buf;
        buf.resize(sizeof(response));
        memcpy(buf.data(),&response,sizeof(response));
        buf.append(public_ip.description.toUtf8());
        client->publish(subscription->topic().filter(),encode(buf),2);
        //添加设备
        device dev;
        dev.ip=bp.ip;
        dev.port=bp.port;
        dev.flag=bp.flag;
        dev.description=description;
        devid_t devid = getIdByDevice(dev);
        clients.insert(devid,dev);
        emit deviceUpdated();
        emit deviceOnline(devid);
        ninfo<<"Device "<<getStringByDeviceId(devid)<<" "<<dev.toFullString()<<"registered online.";
    }
    if(bp.type == DeviceInfo){//收到DeviceInfo，任何情况都添加或更新设备
        device dev;
        dev.ip=bp.ip;
        dev.port=bp.port;
        dev.flag=bp.flag;
        dev.description=description;
        //添加、触发信号
        devid_t devid = getIdByDevice(dev);
        ninfo<<"Device "<<getStringByDeviceId(devid)<<" "<<dev.toFullString()<<"replied.";
        if(!clients.contains(devid)){//没有这个设备
            clients.insert(devid,dev);
            emit deviceUpdated();//必需先触发
            emit deviceOnline(devid);
        }
        else if(clients[devid].toFullString() != dev.toFullString()){//不使用重载的==，因为需要比较附加信息
            clients[devid]=dev;//更新设备
            emit deviceUpdated();//只触发这个
        }
    }
    if(bp.type == RegisterOffline){
        device dev;
        dev.ip=bp.ip;
        dev.port=bp.port;
        if(!clients.contains(getIdByDevice(dev))){//允许使用，因为deviceId只和ip、port有关
            ncritical<<"啊哦，真遗憾，居然等设备下线的时候才知道这个设备存在";
            return;
        }
        clients.remove(getIdByDevice(dev));
        emit deviceUpdated();
        emit deviceOffline(getIdByDevice(dev));
        ninfo<<"Device "<<getStringByDeviceId(getIdByDevice(dev))<<" "<<dev.toFullString()<<"registered offline.";
    }
}


QByteArray Signalling::encode(QByteArray data){
    return Utils::encode(data,password);
}


QByteArray Signalling::decode(QByteArray data){
    return Utils::decode(data,password);
}
