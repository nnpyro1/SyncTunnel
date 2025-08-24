#include "signalling.h" 
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "../../../libary/Qt-AES/qaesencryption.h"
#include <QCryptographicHash>
#include <QDataStream>
#include <QMessageBox>
#include <QThread>


Signalling::Signalling(){
    client = new QMqttClient;
    subscription = nullptr;
}

Signalling::~Signalling(){
    exit();
    client->deleteLater();
}


void Signalling::connectToHost(Communication::ipport host){
    client->setHostname(host.ip);
    client->setPort(host.port);
    client->connectToHost();
    
    QEventLoop loop;
    QTimer timer;
    connect(client,&QMqttClient::connected,&loop,&QEventLoop::quit);
    connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    timer.start(20000);//超时20s
    loop.exec();
    
    if(client->state() != QMqttClient::Connected){
        qCritical()<<"Signalling::connectToHost() Error:Cannot connect to host"<<client->error();
    }
}


QMqttSubscription *Signalling::subscribe(QString topic){
    topicName = topic;
    subscription = client->subscribe(topic,2);
    connect(subscription,&QMqttSubscription::messageReceived,this,&Signalling::on_msg);
    return subscription;
}


void Signalling::setUser(const Communication::ipport user, QString name){
    this->user = user;
    this->user_name = name;
    user_list.clear();
    user_list.append(user);
//    foreach(auto i,user_list)qDebug()<<"Userlist"<<i;
}


void Signalling::setPwd(const QByteArray p){
//    qDebug()<<p;
//    QByteArray temp = p;
    this->pwd = p;
}


QList<Communication::ipport> Signalling::getUserList(){
    //发送消息准备
//    if(subscription)disconnect(subscription,&QMqttSubscription::messageReceived,this,&Signalling::on_msg);//防止意外触发
    /*QJsonObject json;
    json.insert("request","get_user_list");
    json.insert("ip",user.ip);
    json.insert("port",user.port);
    json.insert("user",QString(user_name.toUtf8().toBase64()));
    QByteArray msg = QJsonDocument(json).toJson();
//    qDebug()<<msg;
    
    //加密消息
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd,QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd,QCryptographicHash::Md5);
    QByteArray encode = encription.encode(msg,key,iv).toBase64();
//    qDebug()<<"Signalling::getUserList var:encode ="<<encode;
//    qDebug()<<"Signalling::getUserList msg="<<encription.removePadding(encription.decode(QByteArray::fromBase64(encode),key,iv));
    
    //发送消息
    client->publish(QMqttTopicName(topicName),encode);*/
    
    //发送消息
    send_msg_to_get_user_list(1);
    
    //阻塞等待回复
    is_waiting_userList = true;//设置让on_msg收集用户
    QEventLoop loop;
    QTimer timer;
    connect(this,&Signalling::on_userlist_updata,&loop,&QEventLoop::quit);
    connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    timer.start(2000);
    loop.exec();
    
    
    return user_list;
}


void Signalling::exit(){
    QJsonObject json;
    json.insert("request","disconnect");
    json.insert("ip",user.ip);
    json.insert("port",user.port);
    json.insert("user",QString(user_name.toUtf8().toBase64()));
    json.insert("c",0);
    QByteArray msg = QJsonDocument(json).toJson();
//    qDebug()<<msg;
    
    //加密消息
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd,QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd,QCryptographicHash::Md5);
    QByteArray encode = encription.encode(msg,key,iv).toBase64();
//    qDebug()<<"Signalling::getUserList var:encode ="<<encode;
//    qDebug()<<"Signalling::getUserList msg="<<encription.removePadding(encription.decode(QByteArray::fromBase64(encode),key,iv));
    
    //发送消息
    client->publish(QMqttTopicName(topicName),encode);
    
    //销毁
    QThread::msleep(200);
    subscription->unsubscribe();
    client->disconnectFromHost();
}


void Signalling::send_msg_to_get_user_list(int c){
    QJsonObject json;
    json.insert("request","get_user_list");
    json.insert("ip",user.ip);
    json.insert("port",user.port);
    json.insert("user",QString(user_name.toUtf8().toBase64()));
    json.insert("c",c);
    QByteArray msg = QJsonDocument(json).toJson();
//    qDebug()<<msg;
    
    //加密消息
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd,QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd,QCryptographicHash::Md5);
    QByteArray encode = encription.encode(msg,key,iv).toBase64();
//    qDebug()<<"Signalling::getUserList var:encode ="<<encode;
//    qDebug()<<"Signalling::getUserList msg="<<encription.removePadding(encription.decode(QByteArray::fromBase64(encode),key,iv));
    
    //发送消息
    client->publish(QMqttTopicName(topicName),encode);
}


void Signalling::on_msg(QMqttMessage mqttMsg){
    if(0){
        QMessageBox *b = new QMessageBox;
        b->setText("收到消息");
        b->open();
    }
    
    
    QByteArray encodedMsg = mqttMsg.payload();
//    qDebug()<<"Signalling::on_msg var:encodedMsg ="<<encodedMsg<<QString("(Bytes:%1)").arg(QString(QByteArray::fromBase64(encodedMsg)));
    //解密消息
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd,QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd,QCryptographicHash::Md5);
//    QByteArray decode = QByteArray::fromBase64(encription.removePadding(encription.decode(QByteArray::fromBase64(encodedMsg),key,iv)));
    QByteArray decode = encription.removePadding(encription.decode(QByteArray::fromBase64(encodedMsg),key,iv));
    
//    qDebug()<<"Signalling::on_msg var:decode ="<<decode;
    if(0){
        qDebug()<<"Signalling::on_msg 收到消息"<<(QString)decode;
    }
    
    //解析JSON
    QJsonObject json;
    QJsonDocument jd = QJsonDocument::fromJson(decode);
    if(!jd.isObject()){
        qCritical()<<"Error:Signalling::on_msg : It's not an object";
        return;
    }
    json = jd.object();
    
    //验证用户
    if(QByteArray::fromBase64(json["user"].toString().toUtf8()) != user_name.toUtf8()){
        qCritical()<<QString("Error:Signalling::on_msg : User '%1' is not allowed").arg(QString(QByteArray::fromBase64(json["user"].toString().toUtf8())));
        return;
    }
    else if(json["ip"].toString() == user.ip && (quint16)json["port"].toInt() == user.port){
        qDebug()<<"Good Signalling::on_msg : ignore self";
        return;
    }
    else{
        qDebug()<<"Good Signalling::on_msg : User is allowed";
    }
    
    
     //解析消息
    if(1){//处理用户列表      
        //解析C
        int c = json["c"].toInt();
        if(c > 2){//次数足够就退出
            is_waiting_userList = false;
            //触发信号
            emit on_userlist_updata(user_list);
            return;
        }
        
        if(json["request"] == "get_user_list" && json.contains("ip") && json.contains("port")){//请求用户列表
//            qDebug()<<"进入get_user_list";
            Communication::ipport usr = {json["ip"].toString(),(quint16)json["port"].toInt()};
            if(true/*!user_list.contains(usr)*/ /*&& !(usr == this->user)*/){//不重复
                //调试
                if(0){
                    foreach(auto item,user_list)qDebug()<<item;
                }
                user_list.append(usr);
                //调试
                if(0){
                    foreach(auto item,user_list)qDebug()<<item;
                }
                
                //发送消息准备
                QJsonObject j;
                QByteArray msg_body;
//                QDataStream stream(&msg_body,QIODevice::WriteOnly);
//                stream<<user_list;
                {//遍历并初始化msg_body
                    QJsonArray ary;
                    for(int i=0;i<user_list.size();i++){
                        QJsonObject object;
                        object.insert("ip",user_list[i].ip);
                        object.insert("port",user_list[i].port);
                        ary.append(object);
                    }
                    msg_body = QJsonDocument((ary)).toJson();
                }
//                qDebug()<<"msg_body"<<msg_body;
                j.insert("request","user_list");
                j.insert("body",QString(msg_body.toBase64()));
                j.insert("user",QString(user_name.toUtf8().toBase64()));
                j.insert("ip",user.ip);
                j.insert("port",user.port);
                j.insert("c",c+1);
                QByteArray msg_to_send = QJsonDocument(j).toJson();
                
                //加密消息
                QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
                QByteArray key = QCryptographicHash::hash(pwd,QCryptographicHash::Sha256);
                QByteArray iv = QCryptographicHash::hash(pwd,QCryptographicHash::Md5);
                QByteArray encode = encription.encode(msg_to_send,key,iv).toBase64();
//                qDebug()<<"Signalling::on_msg var:encode ="<<encode;
                
                //发送消息
                client->publish(QMqttTopicName(topicName),encode);
                
                //调试
                if(0){
                    foreach(auto item,user_list)qDebug()<<item;
                }
                
                if(c == 1 && 0){
                    QEventLoop l;
                    QTimer t;
                    connect(&t,&QTimer::timeout,&l,&QEventLoop::quit);
                    t.start(1000);
                    l.exec();
                    send_msg_to_get_user_list(c+1);
                }
            }
        }
        else if(json["request"].toString() == "user_list"){
            qDebug()<<"进入user_list";
//            qDebug()<<"12345";
            QList<Communication::ipport> pendingList;
            {//解析pendingList
                QJsonDocument jd = QJsonDocument::fromJson(QByteArray::fromBase64(json["body"].toString().toUtf8()));
                if(jd.isArray()){
                    QJsonArray ary = jd.array();
                    foreach(auto item,ary){
                        pendingList.append({item["ip"].toString(),(quint16)item["port"].toInt()});
                    }
                }
            }
            if(0)foreach(auto item,pendingList){//调试
                qDebug()<<item;
            }
            
            foreach(auto item,pendingList){//遍历并合并
                if(!user_list.contains(item)){
                    user_list.append(item);
                }
            }
            if(0)foreach(auto item,user_list){//调试
                qDebug()<<item;
            }
            
            if(0){
//                QMessageBox *b = new QMessageBox;
//                b->setText("aaa");
//                b->open();
                foreach(auto item , user_list){
                    qDebug()<<item;
                }
            }
            
        }
        else if(json["request"]=="disconnect"){
            user_list.removeAll({json["ip"].toString(),(quint16)json["port"].toInt()});
        }
        
//        else{
//            qWarning()<<"Warning Signalling::on_msg:Unsupport msg:"<<encode;
//        }
    }
    
    if(0){
        QMessageBox::information(0,"长度",QString::number(user_list.size()));
    }
    //触发信号
    emit on_userlist_updata(user_list);
}
