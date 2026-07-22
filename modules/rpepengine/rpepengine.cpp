#include "rpepengine.h"
#include "qeventloop.h"
#include <core/basic/utils.h>
#include <QUuid>

RpepEngine::RpepEngine(QObject *parent)
    : QObject{parent}
{
    
}


void RpepEngine::setUsername(QString username){
    if(state!=State::Invalid){
        ncritical<<"Unable to set username when state="<<(int)state;
        return;
    }
    this->username = username;
}


void RpepEngine::setPassword(QString pwd){
    if(state!=State::Invalid){
        ncritical<<"Unable to set password when state="<<(int)state;
        return;
    }
    this->pwd = pwd;
}


void RpepEngine::setMqttBroker(ipport broker){
    if(state!=State::Invalid){
        ncritical<<"Unable to set MQTT Broker when state="<<(int)state;
        return;
    }
    this->mqttBroker = broker;
}


Result RpepEngine::init(){
    if(state!=State::Invalid){
        ncritical<<"Unable to init when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    m_communication = new Communication;
    m_signalling = new Signalling;
    //更新状态
    state=State::Connecting;
    
    //1 stun
    emit eventOccurred(Event::GettingPublicIp);
    //多次尝试
    for(int i=0;i<MAX_STUN_RETRIES;i++){
        public_ip = m_communication->stun();
        if(!(ipport{"",0}==public_ip)){
            break;
        }
    }
    //失败后需要使用IPv6
    //暂时保留
    //全部失败后退出
    if((ipport{"",0}==public_ip)){
        state=State::Error;
        emit eventOccurred(Event::Error,{{"error","GettingPublicIp\nFailed to get Public IP."}});
        return Result("GettingPublicIp\nFailed to get Public IP.");
    }
    
    //2 registerOnline
    emit eventOccurred(Event::RegisteringOnline);
    m_signalling->setMqttBroker(mqttBroker.ip,mqttBroker.port);
    m_signalling->setPassport(username,pwd);
    m_signalling->setPublicIp(public_ip);
    m_signalling->start();
    m_signalling->registerOnline();//阻塞注册
    devices = m_signalling->getAllDevices();
    
    
    //绑定信号槽
    connect(m_communication,&Communication::readyRead,this,&RpepEngine::onCommunicationReadyRead);
    connect(m_signalling,&Signalling::deviceOffline,this,[this](devid_t d){
        emit deviceOffline(d);
    });
    connect(m_signalling,&Signalling::deviceOnline,this,[this](devid_t d){
        emit deviceOnline(d);
        auto res = punch(QSet<devid_t>({d}));
        if(!res){
            //不切换状态
        }
    });
    connect(m_signalling,&Signalling::deviceUpdated,this,[this]{
        devices=m_signalling->getAllDevices();
        emit deviceUpdated();
    });
    
    
    //打洞
    auto res=punch(QSet<devid_t>(devices.keyBegin(),devices.keyEnd()));
    if(!res){
        if(res.errorMessage.contains("self")){
            state=State::Error;
            emit eventOccurred(Event::Error,{{"error",res.errorMessage}});
            return res;
        }
    }
    
    //切换状态
    state=State::Ready;
    emit eventOccurred(Event::Ready);
    return Result();
}


void RpepEngine::destroy(){
    if(state==State::Invalid){
        ncritical<<"Unable to destroy when state="<<(int)state;
        return;
    }
}


Result RpepEngine::transfer(QByteArray data, QSet<devid_t> destinations){
    if(state!=State::Ready){
        ncritical<<"Unable to transfer when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
}


void RpepEngine::abortTransfer(){
    if(state!=State::Transferring){
        ncritical<<"Unable to abort transfer when state="<<(int)state;
        return ;
    }
}


Result RpepEngine::sendControl(QByteArray key, QByteArray value, QSet<devid_t> destinations){
    if(state!=State::Ready&&state!=State::Transferring&&state!=State::Busy){
        ncritical<<"Unable to send Control Message when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    QString uuid = QUuid::createUuid().toString(QUuid::Id128);//32个字
    //发送201
    //未完成
}


QString RpepEngine::getUsername(){
    return username;
}


QString RpepEngine::getPassword(){
    return pwd;
}


ipport RpepEngine::getMqttBroker(){
    return mqttBroker;
}


Devices RpepEngine::getAllDevices(){
    return devices;
}


#define tbe(var) var=::qToBigEndian(var)
#define fbe(var) var=::qFromBigEndian(var)
RpepEngine::DataMessageHeader RpepEngine::qFromBigEndian(DataMessageHeader h){
    fbe(h.chunkId);
    fbe(h.totalChunkNum);
    fbe(h.reserved);
    fbe(h.src);
    fbe(h.type);
    fbe(h.version);
    return h;
}

RpepEngine::DataMessageHeader RpepEngine::qToBigEndian(DataMessageHeader h){
    tbe(h.chunkId);
    tbe(h.totalChunkNum);
    tbe(h.reserved);
    tbe(h.src);
    tbe(h.type);
    tbe(h.version);
    return h;
}

RpepEngine::CommonHeader RpepEngine::qFromBigEndian(CommonHeader h){
    fbe(h.reserved);
    fbe(h.src);
    fbe(h.type);
    fbe(h.version);
    return h;
}

RpepEngine::CommonHeader RpepEngine::qToBigEndian(CommonHeader h){
    tbe(h.reserved);
    tbe(h.src);
    tbe(h.type);
    tbe(h.version);
    return h;
}
#undef fbe
#undef tbe


void RpepEngine::send(QByteArray msg, bool e, int d){
    if(e){
        msg=encode(msg);
    }
    int ret=0;
    if(d==-1 || d==-2){
        QSet<devid_t> dsts (devices.keyBegin(),devices.keyEnd());
        if(d==-1)dsts.remove(getIdByDevice(public_ip));
        for(auto dst:dsts){
            ret=m_communication->send(devices.value(dst),msg);
            if(ret<0){
                ncritical<<"发送消息失败 目标"<<getStringByDeviceId(dst);
            }
        }
    }
    else{
        if(!devices.contains(d)){
            ncritical<<"Unknown device "<<d;
            return;
        }
        ret=m_communication->send(devices.value(d),msg);
        if(ret<0){
            ncritical<<"发送消息失败 目标"<<getStringByDeviceId(d);
        }
    }
}


QByteArray RpepEngine::encode(const QByteArray &msg){
    return Utils::encode(msg,pwd);
}


QByteArray RpepEngine::decode(const QByteArray &msg){
    return Utils::decode(msg,pwd);
}


Result RpepEngine::punch(QSet<devid_t> dsts){
    dsts.remove(getIdByDevice(public_ip));
    
    QEventLoop loop;
    QSet<devid_t> received;
    connect(this,&RpepEngine::punchReceived,this,[&](devid_t src,int){//阻塞直到全部收集完
        received.insert(src);
        if(received==dsts){
            loop.quit();
        }
    });
    
    for(auto dst : dsts){
        //发送Punch包
        CommonHeader ch;
        ch.src=getIdByDevice(public_ip);
        ch.type=(quint16)MessageType::Punch;
        QByteArray msg;
        msg.append(getHeaderBytes(ch));
        msg.append(0x01);
        for(int i=0;i<5;i++){
            send(msg,1,dst);
            Utils::multiDelay(1);
        }
    }
    
    //等待
    QTimer::singleShot(5000,&loop,&::QEventLoop::quit);
    loop.exec();
    
    //判断
    auto offlineDevices = dsts;
    offlineDevices-=received;
    if(!offlineDevices.isEmpty()){
        if(offlineDevices.size()<dsts.size()){  //说明有人回复了，证明是未回复设备的问题
            QString offlineString;
            for(auto d:offlineDevices){
                offlineString+=getStringByDeviceId(d)+"\n";
            }
            ncritical<<"Punch failed.\n"<<offlineString;
            unconnectedDevices+=offlineDevices;
            return Result("punch\ndevice\n"+offlineString);
        }
        else{//没人回复，证明是自己的问题
            ncritical<<"Punch failed. No device responsed";
            return Result("punch\nself");
        }
    }
    return Result();
}


void RpepEngine::onCommunicationReadyRead(){
    while(m_communication->hasPendingDatagrams()){
        QByteArray msg = decode(m_communication->readDatagram().data());
        CommonHeader header = getHeaderStruct<CommonHeader>(msg.left(sizeof(header)));
        msg = msg.mid(sizeof(header));
        if(header.version <= MIN_COMPATIBLE_VERSION){
            ncritical<<"Uncompatible version:"<<header.version<<",Current="<<CURRENT_VERSION<<",Min="<<MIN_COMPATIBLE_VERSION;
            continue;
        }
        
        switch((MessageType)header.type){
        case MessageType::Punch:{
            qint8 seq=msg[0];
            qint8 seqToSend = 0;
            if(seq==1){
                seqToSend=2;
            }
            if(seq==2){
                seqToSend=3;
            }
            if(seq==3){
                
            }
            if(seqToSend!=0){
                CommonHeader ch;
                ch.src=getIdByDevice(public_ip);
                ch.type=(quint16)MessageType::Punch;
                QByteArray msg;
                msg.append(getHeaderBytes(ch));
                msg.append(seqToSend);
                send(msg,1,header.src);
            }
            emit punchReceived(header.src,seq);
            emit eventOccurred(Event::Punch,{{"id",getStringByDeviceId(header.src)}});
        }
            
        }
    }
}


RpepEngine::State RpepEngine::getState(){
    return state;
}


template<typename T> QByteArray RpepEngine::getHeaderBytes(T header) {
    static_assert(std::is_trivially_copyable_v<T>,"Type must be a trivially copyable type");
    header=this->qToBigEndian(header);
    QByteArray ret;
    ret.resize(sizeof(header));
    memcpy(ret.data(),&header,sizeof(header));
    return ret;
}


template<typename T>T RpepEngine::getHeaderStruct(const QByteArray msg){
    T ret;
    memcpy(&ret,msg.constData(),sizeof(T));
    ret=this->qFromBigEndian(ret);
    return ret;
}
