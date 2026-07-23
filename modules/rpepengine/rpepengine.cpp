#include "rpepengine.h"
#include "qeventloop.h"
#include <core/basic/utils.h>
#include <QBuffer>
#include <QUuid>
#include <modules/rpepengine/congestioncontrol/congestioncontrol.h>


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


Result RpepEngine::sendControl(QString key, QVariant value, devid_t destination){
    if(state!=State::Ready&&state!=State::Transferring&&state!=State::Busy){
        ncritical<<"Unable to send Control Message when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    QString uuid = QUuid::createUuid().toString(QUuid::Id128);//32个字
    //发送201
    {
        ControlMessageHeader cmh;
        strcpy(cmh.uuid,uuid.toUtf8().constData());
        cmh.keySize=key.toUtf8().size();
        cmh.src=getIdByDevice(public_ip);
        cmh.type=(int)MessageType::ReliableMessage;
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        QDataStream ds(&buf);
        value.save(ds);
        buf.seek(0);
        QByteArray msg = getHeaderBytes(cmh)+key.toUtf8()+buf.readAll();
        bool received = false;
        QEventLoop loop;
        auto conn = connect(this,&RpepEngine::reliableStepsReceived,this,[=,&received,&loop](MessageType mt,QString _uuid){
            if(mt==MessageType::ReliableResponse&&_uuid==uuid){
                received=true;
                loop.quit();
            }
        });
        for(int i=0;i<MAX_RELIABLE_RETRIES;i++){//最大重试6次
            if(!received){
                send(msg,1,destination);
                QTimer::singleShot(RELIABLE_INTERVAL,&loop,&QEventLoop::quit);
                loop.exec();
            }
            else{
                break;
            }
        }
        disconnect(conn);
        if(!received){
            return Result("sendControl\nwaitFor202\ntimeout");
        }
    }
    
    //发送203
    {
        ControlMessageHeader cmh;
        strcpy(cmh.uuid,uuid.toUtf8().constData());
        cmh.keySize = 0;
        cmh.src=getIdByDevice(public_ip);
        cmh.type=(int)MessageType::ReliableDone;
        QByteArray msg = getHeaderBytes(cmh);//203无需携带key,value
        bool received = false;
        QEventLoop loop;
        auto conn = connect(this,&RpepEngine::reliableStepsReceived,this,[=,&received,&loop](MessageType mt,QString _uuid){
            if(mt==MessageType::ReliableComplete&&_uuid==uuid){
                received=true;
                loop.quit();
            }
        });
        for(int i=0;i<MAX_RELIABLE_RETRIES;i++){//最大重试6次
            if(!received){
                send(msg,1,destination);
                QTimer::singleShot(RELIABLE_INTERVAL,&loop,&QEventLoop::quit);
                loop.exec();
            }
            else{
                break;
            }
        }
        disconnect(conn);
        if(!received){
            return Result("sendControl\nwaitFor204\ntimeout");
        }
    }
    return Result();
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
RpepEngine::ControlMessageHeader RpepEngine::qFromBigEndian(ControlMessageHeader h){
    fbe(h.keySize);
    fbe(h.reserved);
    fbe(h.src);
    fbe(h.type);
    fbe(h.version);
    return h;
}

RpepEngine::ControlMessageHeader RpepEngine::qToBigEndian(ControlMessageHeader h){
    tbe(h.keySize);
    tbe(h.reserved);
    tbe(h.src);
    tbe(h.type);
    tbe(h.version);
    return h;
}


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


Result RpepEngine::transferData(QByteArray data, devid_t dst){
    if(state!=State::Ready){
        ncritical<<"Unable to transfer data when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    auto res1 = preloadData(data);
    if(!res1){
        return res1;
    }
    auto res2 = transferPreloadedData(dst);
    return res2;
}


Result RpepEngine::preloadData(QByteArray data){
    if(!transferBuf.isEmpty()){
        ncritical<<"Unable to preload data with a non-empty buffer";
        return Result("preloadData\nbuffer not empty");
    }
    //分片加密并写入缓冲区
    DataMessageHeader dmh;
    dmh.chunkId=0;
    dmh.totalChunkNum = std::ceil(data.size() * 1. / CHUNK_SIZE);
    dmh.src=getIdByDevice(public_ip);
    dmh.type=(int)MessageType::DataPayload;
    transferBuf.resize(dmh.totalChunkNum);
    for(qsizetype i=0;i<data.size();i+=CHUNK_SIZE){
        QByteArray chunk = data.mid(i,CHUNK_SIZE);
        dmh.chunkId=i/CHUNK_SIZE;
        transferBuf[dmh.chunkId]=encode(getHeaderBytes(dmh)+chunk);
    }
    return Result();
}


Result RpepEngine::transferPreloadedData(devid_t dst){
    //1 开始传输
    {
        auto res = sendControl("___START_TRANSFER___","",dst);
        if(!res){
            ncritical<<"Unable to start a transfer."<<res.errorMessage;
            return res;
        }
        QEventLoop loop;
        bool received = false , accepted = false;
        QString refuseReason;
        connect(this,&RpepEngine::transferAccepted,&loop,[&]{
            loop.quit();
            received=true;
            accepted=true;
        });
        connect(this,&RpepEngine::transferRefused,&loop,[&](QString reason){
            loop.quit();
            received=true;
            refuseReason=reason;
        });
        QTimer::singleShot(MAX_TIMEOUT,&loop,&QEventLoop::quit);
        if(!received){
            ncritical<<"No response received for ___START_TRANSFER___";
            return Result("startTransfer\ntimeout");
        }
        if(!accepted){
            nwarning<<"Transfer refused.reason="<<refuseReason;
            return Result("startTransfer\nrefused\n"+refuseReason);
        }
    }
    //2 发送数据包
    CongestionControl::CongestionControlOutput co;
    for(int i=0;i<transferBuf.size();++i){
        send(transferBuf[i],0,dst);
        Utils::multiDelay(1000/co.rate);
        // ### 警告：未完成 ###
    }
}


void RpepEngine::onCommunicationReadyRead(){
    while(m_communication->hasPendingDatagrams()){
        auto datagram = m_communication->readDatagram();
        QByteArray rawMsg = decode(datagram.data());
        if(rawMsg.size()<(qsizetype)sizeof(CommonHeader)){
            ncritical<<"Message Too Short. msg="<<rawMsg;
            continue;
        }
        CommonHeader header = getHeaderStruct<CommonHeader>(rawMsg.left(sizeof(header)));
        QByteArray msg = rawMsg.mid(sizeof(header));
        if(header.version >= MIN_COMPATIBLE_VERSION){
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
            
            break;
        }
            
        case MessageType::ReliableMessage:
        case MessageType::ReliableResponse:
        case MessageType::ReliableDone:
        case MessageType::ReliableComplete:{
            ControlMessageHeader cmh = getHeaderStruct<ControlMessageHeader>(rawMsg);
            QByteArray cmsg = rawMsg.mid(sizeof(cmh));
            MessageType tp=(MessageType)cmh.type;
            if(tp==MessageType::ReliableMessage){
                QByteArray key = cmsg.left(cmh.keySize);
                QBuffer buf;
                buf.open(QBuffer::ReadWrite);
                buf.write(cmsg.mid(cmh.keySize));
                buf.seek(0);
                QDataStream ds(&buf);
                QVariant value;
                value.load(ds);
                //加入去重缓存
                pendingReliableMessages.insert(cmh.uuid,qMakePair(key,value));
                //回复202
                ControlMessageHeader rh;
                rh.keySize=0;
                strcpy(rh.uuid,cmh.uuid);
                rh.src=getIdByDevice(public_ip);
                rh.type=(quint16)MessageType::ReliableResponse;
                send(getHeaderBytes(rh),1,header.src);
            }
            if(tp==MessageType::ReliableResponse||tp==MessageType::ReliableComplete){
                emit reliableStepsReceived(tp,cmh.uuid);
            }
            if(tp==MessageType::ReliableDone){
                //回复204
                ControlMessageHeader rh;
                rh.keySize=0;
                strcpy(rh.uuid,cmh.uuid);
                rh.src=getIdByDevice(public_ip);
                rh.type=(quint16)MessageType::ReliableComplete;
                send(getHeaderBytes(rh),1,header.src);
                //上抛消息
                if(pendingReliableMessages.contains(cmh.uuid)){
                    auto pair = pendingReliableMessages.value(cmh.uuid);
                    QString key=pair.first;
                    QVariant value = pair.second;
                    if(key.startsWith("___")&&key.endsWith("___")){
                        onPrivateControlMessageReceived(key,value,header.src);
                    }
                    else{
                        emit controlReceived(key,value,header.src);
                    }
                    pendingReliableMessages.remove(cmh.uuid);//删除去重缓存
                }
            }
            
            break;
        }
        }
    }
}


void RpepEngine::onPrivateControlMessageReceived(QString key, QVariant value, devid_t src){
    ninfo<<"Control message received.key="<<key<<",value="<<value<<",src="<<getStringByDeviceId(src);
    if(key=="___START_TRANSFER___"){
        if(state==State::Ready){
            sendControl("___ACCEPT_TRANSFER___","",src);
        }
        else{
            sendControl("___REFUSE_TRANSFER___","state="+QString::number((int)state),src);
        }
    }
    if(key=="___ACCEPT_TRANSFER___"){
        emit transferAccepted();
    }
    if(key=="___REFUSE_TRANSFER___"){
        emit transferRefused(value.toString());
    }
}


RpepEngine::State RpepEngine::getState(){
    return state;
}


template<typename T> QByteArray RpepEngine::getHeaderBytes(T header) {
    static_assert(std::is_trivially_copyable_v<T>,"Type must be a trivially copyable type");
    header=RpepEngine::qToBigEndian(header);
    QByteArray ret;
    ret.resize(sizeof(header));
    memcpy(ret.data(),&header,sizeof(header));
    return ret;
}


template<typename T>T RpepEngine::getHeaderStruct(const QByteArray msg){
    T ret;
    memcpy(&ret,msg.constData(),sizeof(T));
    ret=RpepEngine::qFromBigEndian(ret);
    return ret;
}
