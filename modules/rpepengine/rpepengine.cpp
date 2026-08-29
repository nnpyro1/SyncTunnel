#include "rpepengine.h"
#include "qeventloop.h"
#include "qmetaobject.h"
#include <core/basic/utils.h>
#include <QBuffer>
#include <QUuid>
#include <deque>
#include <modules/rpepengine/congestioncontrol/congestioncontrol.h>
#include <QApplication>
#include <algorithm>
#include <QSignalSpy>


RpepEngine::RpepEngine(QObject *parent)
    : QObject{parent}
{
    transferWatchdog.setInterval(TRANSFER_WATCHDOG_TIMEOUT);
    receivingWatchdog.setInterval(RECEIVING_WATCHDOG_TIMEOUT);
    // connect(&receivingWatchdog,&QTimer::timeout,this,[this]{abortReceiving();});//无需联接transferWatchdog因为在transferPreloadedData内连接
    // connect(&receivingReportTimer,&QTimer::timeout,this,[this]{
    //     if(state!=State::Receiving){
    //         receivingReportTimer.stop();
    //         return ;
    //     }
    //     if(receivingBuf.isEmpty()){
    //         nwarning<<"Receiving buf is empty.";
    //         return; 
    //     }
    //     //侦测丢包
    //     chunkid_t start = receivingBuf.lastKey() >REPORT_BATCH?receivingBuf.lastKey()-REPORT_BATCH:0;
    //     QList<chunkid_t> loss;
    //     for(chunkid_t i=start;i<receivingBuf.lastKey();i++){
    //         if(!receivingBuf.contains(i)){
    //             loss.append(i);
    //         }
    //     }
    //     // ninfo<<"Report:loss="<<loss;
    //     // //构造报文
    //     // ReportMessageHeader rmh;
    //     // rmh.src=deviceId;
    //     // rmh.type=(quint16)MessageType::Report;
    //     // rmh.isRttAvailable=/*isRttAvailable*/false;
    //     // rmh.start=start;
    //     // rmh.isEmpty=loss.isEmpty();
    //     // rmh.lastReceive=receivingBuf.lastKey();
    //     // auto e=lastReportElapsedTime.elapsed();
    //     // rmh.deliverRate= e!=0?delivered*1000./e:1;
    //     // if(rmh.deliverRate<1||rmh.deliverRate>10000){
    //     //     nwarning<<"DeliverRate="<<rmh.deliverRate<<" delivered="<<delivered<<" elapsed="<<e;
    //     // }
    //     // QByteArray msgBody;
    //     // for(chunkid_t l:loss){
    //     //     l=::qToBigEndian(l);
    //     //     char lo[sizeof(l)];
    //     //     memcpy(lo,&l,sizeof(l));
    //     //     msgBody.append(lo,sizeof(l));
    //     // }
    //     // chunkid_t start = dmh.chunkId>REPORT_BATCH?dmh.chunkId-REPORT_BATCH:0;
    //     QList<QPair<chunkid_t,chunkid_t>> lossRangeList;
    //     loss.append(UINT_MAX);//用于输出最后一个区间
    //     if(!std::is_sorted(loss.begin(),loss.end())){std::sort(loss.begin(),loss.end());}
    //     for(chunkid_t i=0,last=0,start=0;i<loss.size();i++){
    //         if(loss[i]-last>1){//生成连续闭合区间
    //             lossRangeList.append(qMakePair(start,last));
    //             start=loss[i];
    //         }
    //         last=loss[i];
    //     }
    //     if(!lossRangeList.empty() && lossRangeList[0]==qMakePair(0u,0u)){lossRangeList.pop_front();}//删除第一个[0,0]区间                
    //     ninfo<<"Report(timeout)"/*":loss="<<loss*/<<"lossRangeList"<<lossRangeList;
    //     //构造报文
    //     // ReportMessageHeader rmh;
    //     // rmh.src=deviceId;
    //     // rmh.type=(quint16)MessageType::Report;
    //     // rmh.isRttAvailable=/*isRttAvailable*/true;
    //     // rmh.start=start;
    //     // rmh.isEmpty=loss.isEmpty();
    //     // rmh.lastReceive=dmh.chunkId;
    //     // auto e=lastReportElapsedTime.elapsed();
    //     // rmh.deliverRate= e!=0?delivered*1000./e:1;
    //     ReportMessageHeader rmh;
    //     rmh.src=deviceId;
    //     rmh.type=(quint16)MessageType::Report;
    //     rmh.isRttAvailable=/*isRttAvailable*/false;
    //     rmh.start=start;
    //     rmh.isEmpty=loss.isEmpty();
    //     rmh.lastReceive=receivingBuf.lastKey();
    //     auto e=lastReportElapsedTime.elapsed();
    //     rmh.deliverRate=/* e!=0?delivered*1000./e:1;
    //     if(rmh.deliverRate<1||rmh.deliverRate>10000){
    //         nwarning<<"DeliverRate="<<rmh.deliverRate<<" delivered="<<delivered<<" elapsed="<<e;
    //     }
    //     if(rmh.deliverRate<1||rmh.deliverRate>10000){
    //         nwarning<<"DeliverRate="<<rmh.deliverRate<<" delivered="<<delivered<<" elapsed="<<e;
    //     }*/0;
    //     QByteArray msgBody;
    //     // for(chunkid_t l:loss){
    //     //     l=::qToBigEndian(l);
    //     //     char lo[sizeof(l)];
    //     //     memcpy(lo,&l,sizeof(l));
    //     //     msgBody.append(lo,sizeof(l));
    //     // }
    //     auto insertNum = [&](chunkid_t num){
    //         num=::qToBigEndian(num);
    //         char src[sizeof(num)];
    //         memcpy(src,&num,sizeof(num));
    //         msgBody.append(src,sizeof(src));
    //     };
    //     for(auto range:std::as_const(lossRangeList)){
    //         insertNum(range.first);
    //         insertNum(range.second);
    //     }
    //     //发送
    //     send(getHeaderBytes(rmh)+msgBody,1,acceptableSender);
    //     lastReportElapsedTime.restart();
    //     delivered=0;
    //     lastReportChunk=receivingBuf.lastKey();
    //     int interval = receivingReportTimer.interval();
    //     receivingReportTimer.stop();
    //     receivingReportTimer.start(2*interval);
    // });
}

RpepEngine::~RpepEngine()
{
    
}


void RpepEngine::setUsername(QString username){
    if(state!=State::Invalid){
        ncritical<<"Unable to set username when state="<<(int)state;
        return;
    }
    this->username = username;
    
    //由于不知道setUsername和setPassword的调用顺序，preporcessedKey将会被赋值两次，其中最后一次的一定是正确的    
    preprocessedKey=Utils::getSafePassword(username,pwd);    
}


void RpepEngine::setPassword(QString pwd){
    if(state!=State::Invalid){
        ncritical<<"Unable to set password when state="<<(int)state;
        return;
    }
    this->pwd = pwd;
    // preprocessedKey=QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
    
    //由于不知道setUsername和setPassword的调用顺序，preporcessedKey将会被赋值两次，其中最后一次的一定是正确的
    preprocessedKey=Utils::getSafePassword(username,pwd);
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
        emit errorOccurred(Error::PublicIp);
        return Result("GettingPublicIp\nFailed to get Public IP.");
    }
    deviceId = getIdByDevice(public_ip);
    
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
    connect(m_signalling,&Signalling::deviceUpdated,this,[]{
        ninfo<<"Device List Updated";
    });
    connect(m_signalling,&Signalling::deviceUpdated,this,[this]{
        devices=m_signalling->getAllDevices();
        emit deviceUpdated();
        QString dev;
        foreach(auto d,devices.keys())dev.append(getStringByDeviceId(d)+"\n");
        ninfo<<"设备列表已更新\n"<<dev;
    });
    
    //2 registerOnline
    emit eventOccurred(Event::RegisteringOnline);
    m_signalling->setMqttBroker(mqttBroker.ip,mqttBroker.port);
    m_signalling->setPassport(username,pwd);
    m_signalling->setPublicIp(public_ip);
    m_signalling->start();
    m_signalling->registerOnline();//阻塞注册
    devices = m_signalling->getAllDevices();
    
    
    //打洞
    auto res=punch(QSet<devid_t>(devices.keyBegin(),devices.keyEnd()));
    if(!res){
        if(res.errorMessage.contains("self")){
            state=State::Error;
            emit eventOccurred(Event::Error,{{"error",res.errorMessage}});
            emit errorOccurred(Error::Punch);
            return res;
        }
    }
    
    //切换状态
    state=State::Ready;
    emit eventOccurred(Event::Ready);
    timer_keepAlive.setSingleShot(false);
    timer_keepAlive.start(KEEPALIVE_INTERVAL);
    connect(&timer_keepAlive,&QTimer::timeout,this,[this]{
        CommonHeader h;h.type=(int)MessageType::KeepAlive;h.src=deviceId;send(getHeaderBytes(h));
        m_communication->keepAlive();
    });
    return Result();
}


void RpepEngine::destroy(){
    if(state==State::Invalid){
        ncritical<<"Unable to destroy when state="<<(int)state;
        return;
    }
    if(state==State::Transferring){
        abortTransfer();
    }
    if(state==State::Receiving){
        abortReceiving();
    }
    timer_keepAlive.stop();
    if(m_signalling){
        m_signalling->registerOffline();
        m_signalling->stop();
        m_signalling->deleteLater();
        m_signalling=nullptr;
    }
    if(m_communication){
        m_communication->deleteLater();
        m_communication = nullptr;
    }
}


Result RpepEngine::transfer(FileByteArray data, QSet<devid_t> destinations){
    if(state!=State::Ready){
        ncritical<<"Unable to transfer when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    //1 规划
    {
        Devices dsts;
        for(auto id:destinations){
            dsts.insert(id,devices[id]);
        }
        auto senders=dsts;
        senders.insert(deviceId,public_ip);
        auto plan=planAutoSend(dsts);
        //转换数据格式
        QMap<devid_t,QByteArray> tasks;
        for(auto sender:senders){
            QList<devid_t> task;
            for(auto i:plan){
                for(auto j:i){
                    if(j.first==sender){
                        task.append(getIdByDevice(j.second));
                    }
                }
            }
            if(sender==public_ip){//是自己就直接传输
                // transferTaskQueue=QQueue<devid_t>(task);
                transferTaskQueue.clear();
                foreach(auto i,task){
                    transferTaskQueue.enqueue(i);
                }
            }
            QByteArray pd;
            for(auto i:task){
                char c[sizeof(i)];
                i=::qToBigEndian(i);
                memcpy(c,&i,sizeof(i));
                pd.append(c,sizeof(i));
            }
            tasks.insert(getIdByDevice(sender),pd);
        }
        //发送规划
        for(auto sender:senders){
            if(sender==public_ip){//自己：已经直接添加到发送队列中，不操作
                
            }
            else{
                auto senderId = getIdByDevice(sender);
                ninfo<<"Plans for "<<getStringByDeviceId(senderId)<<" are "<<tasks[senderId];
                sendControl("___TASK___",tasks[senderId],senderId);
            }
        }
        //执行自己的任务
        return transferData(std::move(data),transferTaskQueue.dequeue());
    }
}


void RpepEngine::abortTransfer(){
    if(state!=State::Transferring){
        ncritical<<"Unable to abort transfer when state="<<(int)state;
        return ;
    }
    //1 停止发送任何数据包
    emit transferAborted();
    QThread::msleep(50);
    //2 发送消息
    sendControl("___ABORT_TRANSFER___","",transferDestination);
    //3 清除状态
    // QTimer::singleShot(2000,this,[this]{
        // transferTaskQueue.clear();
        // transferBuf.clear();
        // transferDestination = 0;
        // transferWatchdog.stop();
        senderReset();
    transferBuf.clear();
    // });
}


Result RpepEngine::sendControl(QString key, QVariant value, devid_t destination){
    if(state!=State::Ready&&state!=State::Transferring&&state!=State::Busy&&state!=State::Receiving){
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


device RpepEngine::getPublicIp(){
    return public_ip;
}


bool RpepEngine::acquireBusy(){
    if(state==State::Ready){
        state=State::Busy;
        return true;
    }
    return false;
}

Result RpepEngine::externalSend(QByteArray data, bool e, int d){
    if(state==State::Busy){
        CommonHeader h;
        h.src=deviceId;
        h.type=(int)MessageType::External;
        send(getHeaderBytes(h)+data,1,d);
        return Result();
    }
    else{
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
}

void RpepEngine::releaseBusy(){
    if(state==State::Busy){
        state=State::Ready;
    }
}


void RpepEngine::reset(){
    if(state==State::Invalid || state==State::Error || state==State::Connecting){
        ncritical<<"Unacceptable state to reset";
        return;
    }
    state=State::Ready;
    transferBuf.clear();
    transferWatchdog.stop();
    transferDestination = 0;
    // receivingBuf.clear();
    lastReportElapsedTime.invalidate();
    lastReportChunk=-1;
    acceptableSender=0;
    receivingWatchdog.stop();
    receivingReportTimer.stop();
    delivered=0;
    transferTaskQueue.clear();
}


#define tbe(var) var=::qToBigEndian(var)
#define fbe(var) var=::qFromBigEndian(var)
RpepEngine::ReportMessageHeader RpepEngine::qFromBigEndian(ReportMessageHeader h){
    fbe(h.isEmpty);
    fbe(h.isRttAvailable);
    fbe(h.start);
    fbe(h.reserved);
    fbe(h.src);
    fbe(h.type);
    fbe(h.version);
    fbe(h.lastReceive);
    fbe(h.deliverRate);
    return h;
}

RpepEngine::ReportMessageHeader RpepEngine::qToBigEndian(ReportMessageHeader h){
    tbe(h.isEmpty);
    tbe(h.isRttAvailable);
    tbe(h.start);
    tbe(h.reserved);
    tbe(h.src);
    tbe(h.type);
    tbe(h.version);
    tbe(h.lastReceive);
    tbe(h.deliverRate);
    return h;    
}

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
            ncritical<<"Unknown device "<<getStringByDeviceId(d);
            return;
        }
        ret=m_communication->send(devices.value(d),msg);
        if(ret<0){
            ncritical<<"发送消息失败 目标"<<getStringByDeviceId(d)<<" 返回"<<ret<<" 实际"<<msg.size();
        }
    }
}


QByteArray RpepEngine::encode(const QByteArray &msg){
    // return Utils::encode(msg,pwd);
    return Utils::encodeRaw(msg,preprocessedKey);
}


QByteArray RpepEngine::decode(const QByteArray &msg){
    // return Utils::decode(msg,pwd);
    return Utils::decodeRaw(msg,preprocessedKey);
}


Result RpepEngine::punch(QSet<devid_t> dsts){
    dsts.remove(getIdByDevice(public_ip));
    
    QEventLoop loop;
    QSet<devid_t> received;
    connect(this,&RpepEngine::punchReceived,&loop,[&](devid_t src,int){//阻塞直到全部收集完
        // received.count();//测试代码。之后删除。
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
    if(!dsts.empty()){//不空
        QTimer::singleShot(5000,&loop,&::QEventLoop::quit);
        loop.exec();
    }
    
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


Result RpepEngine::transferData(FileByteArray data, devid_t dst){
    if(state!=State::Ready){
        ncritical<<"Unable to transfer data when state="<<(int)state;
        return Result("StateCheck\nstate="+QString::number((int)state));
    }
    auto res1 = preloadData(std::move(data));
    if(!res1){
        return res1;
    }
    auto res2 = transferPreloadedData(dst);
    return res2;
}


Result RpepEngine::preloadData(FileByteArray data){
    if(transferBuf.getState()!=transferBuf.Invalid){
        ncritical<<"Unable to preload data with a non-empty buffer";
        return Result("preloadData\nbuffer not empty");
    }
    if(data.isEmpty()){
        ncritical<<"Unable to preload data with a empty data";
        return Result("preloadData\nempty data");
    }
    //分片加密并写入缓冲区
    DataMessageHeader dmh;
    dmh.chunkId=0;
    dmh.totalChunkNum = (data.size()+(CHUNK_SIZE-1))/CHUNK_SIZE;
    dmh.src=getIdByDevice(public_ip);
    dmh.type=(int)MessageType::DataPayload;
    // transferBuf.resize(dmh.totalChunkNum);
    const int moreSize = sizeof(dmh)
#ifndef DEBUG_NO_ENCRYPTION
                         +crypto_aead_xchacha20poly1305_ietf_NPUBBYTES+crypto_aead_xchacha20poly1305_ietf_ABYTES
#endif
        ;
    transferBuf.init(CHUNK_SIZE+moreSize);
    for(qsizetype i=0;i<data.size();i+=CHUNK_SIZE){
        QByteArray chunk = data.mid(i,CHUNK_SIZE);
        qsizetype paddingSize=0;
        lastChunkSize=chunk.size()+moreSize;
        // if(chunk.size()<CHUNK_SIZE){//最后一个
        //     paddingSize = CHUNK_SIZE-chunk.size();//加密前的差值等于加密后的差值
        //     lastChunkSize=chunk.size()+moreSize;
        //     // ndb<<"最后一个处理\npaddingSize"
        // }
        dmh.chunkId=i/CHUNK_SIZE;
        // // transferBuf[dmh.chunkId]=encode(getHeaderBytes(dmh)+chunk);
        // auto tmp=encode(getHeaderBytes(dmh)+chunk)+QByteArray(paddingSize,0x5B);
        
        if(chunk.isEmpty()){
            ncritical<<"Chunk #"<<dmh.chunkId<<"is empty";
        }
        auto encoded = encode(getHeaderBytes(dmh)+chunk);
        if(encoded.size()< CHUNK_SIZE+moreSize){//最后一个需要填充
            lastChunkSize=encoded.size();
            ndb<<"encoded.size="<<encoded.size();
            auto needsPadding = CHUNK_SIZE+moreSize-encoded.size();
            ndb<<"needsPadding="<<needsPadding;
            encoded.append(QByteArray(needsPadding,0x5B));
        }
        
        transferBuf.append(encoded);
        if(dmh.chunkId%7==0){
            emit eventOccurred(Event::Preloading,{{"i",dmh.chunkId},{"total",dmh.totalChunkNum}});
        }
    }
    transferBuf.finalize();
    transferTotalSize=/*(data.size()+(CHUNK_SIZE-1))/CHUNK_SIZE*/dmh.totalChunkNum;
    ninfo<<"transferTotalSize:"<<transferTotalSize;
    if(transferTotalSize==0){
        return Result("preloadData\ndata too large");//发生回绕
    }
    return Result();
}


Result RpepEngine::transferPreloadedData(devid_t dst){
    //1 开始传输
    transferDestination = dst;
    state=State::Transferring;
    bool isAborted = false;
    QObject useless;
    connect(this,&RpepEngine::transferAborted,&useless,[&isAborted]{isAborted=true;});//侦测是否中断。随便绑定一个同作用域的QObject
    {
        auto res = sendControl("___START_TRANSFER___",transferTotalSize,dst);
        if(!res){
            ncritical<<"Unable to start a transfer."<<res.errorMessage;
            senderReset();
            state=State::Ready;
            emit errorOccurred(Error::StartTransfer);
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
        loop.exec();
        if(isAborted){
            ninfo<<"传输被强制终止";
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            else{
                QMetaObject::invokeMethod(this,[this]{transferPreloadedData(transferTaskQueue.dequeue());},Qt::QueuedConnection);
            }
            emit eventOccurred(Event::TransferAborted);
            return Result();
        }
        if(!received){
            ncritical<<"No response received for ___START_TRANSFER___";
            state=State::Ready;
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            emit errorOccurred(Error::StartTransfer);            
            return Result("startTransfer\ntimeout");
        }
        if(!accepted){
            nwarning<<"Transfer refused.reason="<<refuseReason;
            // state=State::Ready;
            senderReset();
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            return Result("startTransfer\nrefused\n"+refuseReason);
        }
    }
    
    //2 发送数据包
    // Utils::multiDelay(120);//玄学等待
    using namespace std;
    CongestionControl cc;
    CongestionControl::CongestionControlOutput ccoutput/* = {INITIAL_RATE}*/;
    CongestionControl::CongestionControlInput ccinput;
    // QMap<chunkid_t,double> elapsedTimes;
    QElapsedTimer timer;
    timer.start();
    QHash<chunkid_t,chunkid_t> retransferIgnore;//key存储忽略的重传的包，value存储从表中删除项目时需要的Report end
    deque<chunkid_t> tq;
    QSet<chunkid_t> tqSet;
    // ccinput.totalChunks=transferBuf.size()-1;
    // QElapsedTimer lastReportTimer;
    QTimer reportReceiveTimer;
    int reportLossCount = 0;
    auto conn = connect(this,&RpepEngine::reportReceived,&useless,[&](ReportMessageHeader report,QList<chunkid_t> loss){//随便用一个uselessc的变量管理生命周期
        transferWatchdog.stop();transferWatchdog.start();//重置看门狗
        // ndb<<"Report received. loss="<<loss;
        // if(!std::is_sorted(loss.begin(),loss.end())){
        //     std::sort(loss.begin(),loss.end());
        // }
        // for(auto i=loss.rbegin();i<loss.rend();++i){
        //     if(!tq.contains(*i)){
        //         if(retransferIgnore.contains(*i) && retransferIgnore[*i]>report.lastReceive){//过了一个rtt了，允许重传
        //             retransferIgnore.remove(*i);
        //         }
        //         if(!retransferIgnore.contains(*i)){
        //             // tq.insert(0,*i);
        //             tq.prepend(*i);//重传
        //             retransferIgnore.insert(*i,ccinput.chunkId);//一个RTT内不再重传
        //         }
        //         else{
        //             // ninfo<<"Retransfer of Data #"<<*i<<" was ignored.";
        //         }
        //     }
        // }
        //性能优化 性能筛查通过，中性能50us
        QList<chunkid_t> retransferList;
        retransferList.resize(loss.size());
        qsizetype retransferSize=0;
        chunkid_t *retransferListData = retransferList.data();
        const chunkid_t *lossData = loss.constData();
        auto lossSize = loss.size();
        for(qsizetype i=0;i<lossSize;i++){
            chunkid_t l = lossData[i];
            if(!tqSet.contains(l)){//仅在不存在时插入
                bool cts = retransferIgnore.contains(l);
                bool ent = cts && retransferIgnore[l]>report.lastReceive;
                if(ent){
                    retransferIgnore.remove(l);
                }
                if(!cts || retransferIgnore[l]>report.lastReceive){
                    retransferListData[retransferSize++]=l;
                    tqSet.insert(l);
                    retransferIgnore.insert(l,ccinput.chunkId);
                }
            }
        }
        retransferList.resize(retransferSize);
        tq.insert(tq.begin(),retransferList.cbegin(),retransferList.cend());
        
        //从此到结束存在性能问题（已解决）
        ccinput.loss=loss;
        if(report.isRttAvailable)ccinput.rtt=timer.nsecsElapsed()/1.e6-ccinput.elapsedTimes.value(report.lastReceive);//此处elapsedTimes的性能问题已解决
        ccinput.start=report.start;
        ccinput.end=report.lastReceive;
        // if(lastReportTimer.isValid())ccinput.timeToLastReport=lastReportTimer.elapsed();
        // lastReportTimer.restart();
        // ccinput.elapsedTimes=elapsedTimes;
        // ccinput.deliverRate=report.deliverRate;
        //性能问题结束
        if(report.deliverRate>2){//存在deliverRate信息
            ccinput.deliverRate = ccinput.deliverRate==0? report.deliverRate
                                                           :report.deliverRate*DELIVER_RATE_EWMA_WEIGHT+ccinput.deliverRate*(1-DELIVER_RATE_EWMA_WEIGHT);
            
        }
        //调用
        QElapsedTimer tmpPerfTimer;tmpPerfTimer.start();
        cc.update(ccinput);
        auto usecs = tmpPerfTimer.nsecsElapsed()/1.e3;
        if(usecs>/*50*/10){
            ndb<<"Performance microseconds:"<<usecs;
        }
        ccoutput=cc.getOutput();
        //更新信号
        //性能存疑，但不是瓶颈
        emit congestionControlInfoUpdated(ccinput,ccoutput);
        ccinput.lastEnd=ccinput.end;
        ccinput.lastSend=ccinput.chunkId;
        // reportReceiveTimer.stop();
        reportReceiveTimer.start(MAX_SAFE_NOSEND * MAX_REPORT_OFFSET / qMin(ccoutput.rate,ccinput.deliverRate>2?ccinput.deliverRate:ccoutput.rate) * 1000 + qMax(ccoutput.dcong,ccinput.rtt));
        reportLossCount=0;
    });
    // connect(this,&RpepEngine::transferAborted,&cc,[&isAborted]{isAborted=true;});//侦测是否中断。随便绑定一个同作用域的QObject//已在开头添加
    connect(&transferWatchdog,&QTimer::timeout,&cc,[this]{abortTransfer();});
    connect(&reportReceiveTimer,&QTimer::timeout,this,[&]{
        if(state!=State::Transferring){
            ncritical<<"state="<<(int)state;
            reportReceiveTimer.stop();//停止以防重新触发
            return;
        }
        reportLossCount++;
        ccoutput.rate/=2.;//速率除以2
        ninfo<<"Report长时间未接收，速率为"<<ccoutput.rate;
        auto co = ccoutput;
        co.stateKeep=-1234;//用来做区分，不是真正的拥塞控制运行
        emit congestionControlInfoUpdated(ccinput,co);
        reportReceiveTimer.stop();
        if(reportLossCount>=5){//阻塞等待Report
            bool succeeded=false;
            for(int i=0;i<5;i++){
                //构造消息
                CommonHeader header;
                header.src=deviceId;
                header.type=(quint16)MessageType::RequestReport;
                //发送消息
                if(isAborted){
                    return;
                }
                send(getHeaderBytes(header),1,dst);
                //阻塞等待
                QEventLoop loop;
                QTimer::singleShot(2*qMax(ccoutput.dcong,ccinput.rtt),&loop,&QEventLoop::quit);//timer需要考虑dcong=0的特殊情况
                connect(this,&RpepEngine::reportReceived,&loop,[&]{loop.quit();succeeded=true;});
                loop.exec();
                if(succeeded){break;}
            }
            if(!succeeded){
                ncritical<<"客户端"<<getStringByDeviceId(dst)<<"下线"; //#####错误待处理！！！！！#####
            }
        }
        reportReceiveTimer.start(MAX_SAFE_NOSEND * MAX_REPORT_OFFSET / qMin(ccoutput.rate,ccinput.deliverRate) * 1000 + qMax(ccoutput.dcong,ccinput.rtt));
    });
    for(int i=0;i<transferTotalSize;i++){
        tq.push_back(i);
        tqSet.insert(i);
    }
    transferWatchdog.start();
    ccinput.totalChunks=transferTotalSize;
    ccinput.lastEnd = 0;
    ccinput.elapsedTimes.reserve(transferTotalSize);
    auto sendUntilTqIsEmpty = [&]{
        while(!tq.empty()){
            if(isAborted){
                ninfo<<"传输被强制终止";
                if(transferTaskQueue.empty()){
                    transferBuf.clear();
                }
                emit eventOccurred(Event::TransferAborted);
                // return Result();
                return;
            }
            int i=tq.front();
            tq.pop_front();
            tqSet.remove(i);
            // ccinput.chunkId=i;
            //转换rate
            quint64 db;
            char tmp[sizeof(double)];double drate=ccoutput.fullrate<=2?ccoutput.rate:qMin(ccoutput.rate,ccoutput.fullrate);
            memcpy(&db,&drate,sizeof(double));
            db=::qToBigEndian(db);
            memcpy(tmp,&db,sizeof(double));
            QByteArray data=transferBuf.read(i);
            if(i==transferTotalSize-1){//最后一个包特殊处理
                auto ls = data.size();
                data=data.left(lastChunkSize);
                ndb<<"最后一个包特殊处理,size="<<data.size()<<"截取前size="<<ls<<"lastChunkSize="<<lastChunkSize;
            }
            send(data/*transferBuf[i]*//*+QByteArray(tmp,sizeof(double))*//*+"DRAT_TP_"*/,0,dst);
            // ndb<<"Data #"<<i<<" sent.";
            ccinput.chunkId = i;
            ccinput.elapsedTimes.insert(i,timer.nsecsElapsed()/1.e6);
            Utils::multiDelay(1000/ccoutput.rate,/*[]{QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);}*/nullptr);
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,100);
            // ndb<<"rate:"<<ccoutput.rate;
        }
    };
    
    sendUntilTqIsEmpty();
    if(isAborted){
        return Result();
    }
    
    //-----
    //3 重传
    while(1){
        //准备发送消息
        transferWatchdog.stop();
        transferWatchdog.start();
        QEventLoop loop;
        bool isCompleted = false;
        connect(this,&RpepEngine::transferCompleted,&loop,[&]{
            loop.quit();
            ninfo<<"传输完成";
            isCompleted=true;
        });
        connect(this,&RpepEngine::retransferRequested,&loop,[&](QSet<chunkid_t> loss){
            loop.quit();
            if(isAborted){
                return;//此时在loop.exec处退出，然后退出函数
            }
            QList<chunkid_t> sortedLoss(loss.constBegin(),loss.constEnd());
            std::sort(sortedLoss.begin(),sortedLoss.end());
            for(auto i:sortedLoss){
                if(isAborted){
                    abortTransfer();//不需要管是否sendControl失败，因为程序将要退出，接收方看门狗会解决一切
                    return;
                }
                // send(transferBuf[i],0,dst);
                // ninfo<<"Data #"<<i<<"Retransferred.";
                // QThread::msleep(50);
                // QApplication::processEvents();
                // transferWatchdog.stop();
                // transferWatchdog.start();    
            }
            tq=deque<chunkid_t>(sortedLoss.begin(),sortedLoss.end());
            tqSet.clear();
            tqSet=QSet<chunkid_t>(sortedLoss.begin(), sortedLoss.end());
            sendUntilTqIsEmpty();//快速重传
            if(isAborted){
                return;
            }
            
            auto res=sendControl("___FINISH_TRANSFER___",(transferTotalSize),dst);
            if(!res){
                ncritical<<"Unable to finish transfer";
                // transferBuf.clear();
                // transferDestination = 0;
                // state=State::Ready;
                // transferWatchdog.stop();
                abortTransfer();
                senderReset();
                if(transferTaskQueue.empty()){
                    transferBuf.clear();
                }
                else{
                    QMetaObject::invokeMethod(this,[this]{transferPreloadedData(transferTaskQueue.dequeue());},Qt::QueuedConnection);
                }
                emit errorOccurred(Error::FinishTransfer);
            }
        },Qt::QueuedConnection);
        
        //发送消息
        auto res=sendControl("___FINISH_TRANSFER___",(transferTotalSize),dst);
        if(!res){
            ncritical<<"Unable to finish transfer";
            // transferBuf.clear();
            // transferDestination = 0;
            // state=State::Ready;
            // transferWatchdog.stop();
            abortTransfer();
            senderReset();
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            else{
                QMetaObject::invokeMethod(this,[this]{transferPreloadedData(transferTaskQueue.dequeue());},Qt::QueuedConnection);
            }
            return res;
        }
        //等待结果
        if(!isCompleted){loop.exec();}
        if(isAborted){
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            emit errorOccurred(Error::FinishTransfer);            
            return Result();
        }
        if(isCompleted){//发送完成
            ndb<<"传输完成,状态重置";
            //清除除了transferBuf以外的所有状态
            // state=State::Ready;
            senderReset();
            
            //从队列中取出任务
            if(!transferTaskQueue.isEmpty()){
                QMetaObject::invokeMethod(this,[this]{transferPreloadedData(transferTaskQueue.dequeue());});//在事件循环中运行
            }
            else{
                transferBuf.clear();
                transferDestination = 0;
                transferWatchdog.stop();
                senderReset();
            }
            // if(transferTaskQueue.empty()){
            //     transferBuf.clear();
            // }
            return Result();
        }
    }
}


QVector<QVector<QPair<ipport, ipport> > > RpepEngine::planAutoSend(Devices dsts){
    QQueue<ipport> senders;
    QQueue<ipport> receivers;
    QVector<QVector<QPair<ipport,ipport>>> result;
    foreach(ipport i,dsts)receivers.append(i);
    senders.append(public_ip);
    
    //开始规划
    ninfo<<"开始规划发送表";
    while(!receivers.empty()){//规划到没有可用的接收者了
        QQueue<ipport> new_senders;
        QVector<QPair<ipport,ipport>> thisRound;
        ninfo<<"----------";
        foreach(ipport sender , senders){
            if(receivers.empty())break;
            thisRound.append(QPair<ipport,ipport>(sender,receivers.front()));
            ninfo<<"配对："<<sender<<" "<<receivers.front();
            new_senders.append(receivers.front());
            receivers.pop_front();
        }
        senders.append(new_senders);
        result.append(thisRound);
    }
    ninfo<<"结果："<<(result);
    return result;
}


void RpepEngine::abortReceiving(){
    if(state!=State::Receiving){
        ncritical<<"Unable to abort receiving when state="<<(int)state;
        return;
    }
    
    //1 发送消息
    sendControl("___ABORT_RECEIVING___","",acceptableSender);
    
    //2 清除状态
    // receivingBuf.clear();
    // lastReportElapsedTime.invalidate();
    // lastReportChunk=-1;
    // acceptableSender=0;
    // receivingWatchdog.stop();
    // receivingReportTimer.stop();
    // delivered=0;
    receiverReset();
}


void RpepEngine::receiverReset(){
    if(state==State::Receiving){
        // receivingBuf.clear();
        lastReportElapsedTime.invalidate();
        lastReportChunk=0;
        acceptableSender=0;
        receivingWatchdog.stop();
        receivingReportTimer.stop();
        delivered=0;
        state=State::Ready;
    }
    else{
        ncritical<<"Unable to reset receiver when state="<<(int)state;
    }
}


void RpepEngine::senderReset(){
    if(state==State::Transferring){
        transferDestination=0;
        transferWatchdog.stop();
        transferTotalSize=0;
        state=State::Ready;
    }
    else{
        ncritical<<"Unable to reset sender when state="<<(int)state;
    }
}

QByteArray RpepEngine::generateLossRange(chunkid_t startReport, chunkid_t endReport){
    if(state!=State::Receiving){
        ncritical<<"Unable to generate lossRange when state="<<(int)state;
        return {};
    }
    
    QList<QPair<chunkid_t,chunkid_t>> lossRangeList;
    qint64 start=-1;
    auto &receivingBufSize = endReport;
    for(chunkid_t i=startReport;i<receivingBufSize;++i){
        if(receivingBuf.hasValue(i)){//收到
            if(start>=0){//需要闭合区间
                lossRangeList.append(qMakePair(start,i-1));
                start=-1;
            }
        }
        else{//丢失
            if(start<0){//设置start
                start=i;
            }
        }
        
        //添加最后一个
        if(i==receivingBufSize-1 && start>=0){
            lossRangeList.append(qMakePair(start,i));
        }
    }
    
    //性能优化插入
    QByteArray ret;
    ret.resize(lossRangeList.size()*2*sizeof(chunkid_t));
    auto data = ret.data();
    auto insert = [data](qsizetype i,chunkid_t num){
        num=::qToBigEndian(num);
        memcpy(data+i*sizeof(chunkid_t),&num,sizeof(chunkid_t));
    };
    qsizetype i=0;
    for(auto p:lossRangeList){
        insert(i++,p.first);
        insert(i++,p.second);
    }
    
    return ret;
}


void RpepEngine::onCommunicationReadyRead(){
    int datagram_cnt=0;
    while(m_communication->hasPendingDatagrams()){
        datagram_cnt++;
#ifdef NNPYRO_PERFORMANCE_ANALYSIS
        QElapsedTimer performanceTimer;
        performanceTimer.start();
#endif
        auto datagram = m_communication->readDatagram();
        QByteArray undecoded = datagram.data();
        double rate=0;
        if(undecoded.endsWith("_TP_")){//尾部未加密载荷
            undecoded.chop(4);
            QByteArray id = undecoded.right(4);
            undecoded.chop(4);
            if(id=="DRAT"){
                QByteArray rt = undecoded.right(sizeof(double));
                undecoded.chop(sizeof(double));
                quint64 tmp;
                memcpy(&tmp,rt.constData(),sizeof(double));
                //翻转
                tmp=::qFromBigEndian(tmp);
                memcpy(&rate,&tmp,sizeof(double));
            }
        }
        QByteArray rawMsg = decode(undecoded);
        if(rawMsg.size()<(qsizetype)sizeof(CommonHeader)){
            ncritical<<"Message Too Short. msg="<<rawMsg;
            continue;
        }
        CommonHeader header = getHeaderStruct<CommonHeader>(rawMsg.left(sizeof(header)));
        QByteArray msg = rawMsg.mid(sizeof(header));
        if(header.version < MIN_COMPATIBLE_VERSION || header.version > CURRENT_VERSION){
            ncritical<<"Uncompatible version:"<<header.version<<",Current="<<CURRENT_VERSION<<",Min="<<MIN_COMPATIBLE_VERSION;
            continue;
        }
        
        //解密开销30us，下面代码目标50us
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
            // ndb<<"Reliable receive "<<header.type;
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
                        /*RUN_LATER*/({onPrivateControlMessageReceived(key,value,header.src);});
                    }
                    else{
                        emit controlReceived(key,value,header.src);
                    }
                    pendingReliableMessages.remove(cmh.uuid);//删除去重缓存
                }
            }
            break;
        }
        case MessageType::RequestReport:
        case MessageType::DataPayload:{
            if(state!=State::Receiving){
                ncritical<<"Received DataPayload when state="<<QString::number((int)state);
                break;
            }
            if(header.src!=acceptableSender){
                ncritical<<"Unacceptable sender:"<<header.src<<",accept "<<getStringByDeviceId(acceptableSender);
                break;
            }
            chunkid_t chunkId;
            if(header.type==(int)MessageType::RequestReport){
                // chunkId=receivingBuf.lastKey();
                //遍历找到最后一个有效
                chunkId=0;
                if(receivingBuf.size()==0){
                    chunkId=0;
                }
                else for(qint64 i=receivingBuf.size()-1;i>=0;--i){
                        if(receivingBuf.hasValue(i)){
                            chunkId=i;
                            break;
                        }
                }
            }
            if(header.type==static_cast<int>(MessageType::DataPayload)){
                //加入接收缓冲区
                DataMessageHeader dmh;
                if(rawMsg.size()<(qsizetype)sizeof(dmh)){
                    ncritical<<"Message too short:"<<rawMsg;
                    break;
                }
                dmh=getHeaderStruct<DataMessageHeader>(rawMsg);
                chunkId=dmh.chunkId;
                QByteArray payload = rawMsg.mid(sizeof(dmh));
                // receivingBuf.insert(dmh.chunkId,payload);
                // if(receivingBuf.size()<=dmh.chunkId){
                    // receivingBuf.resize(dmh.totalChunkNum);
                // }
                // receivingBuf[dmh.chunkId]=payload;
                receivingBuf.write(dmh.chunkId,payload);
                // ndb<<"Data #"<<dmh.chunkId<<" received.";
                if(!lastReportElapsedTime.isValid()){
                    lastReportElapsedTime.start();
                }
                emit receivingProgressUpdated(dmh.chunkId,dmh.totalChunkNum);
                //增加交付计数
                delivered++;
                //重置定时器
                // receivingReportTimer.stop();
                // if(rate!=0){
                //     receivingReportTimer.start(MAX_SAFE_NOSEND*1000./rate);
                //     ninfo<<"启动接收超时定时器："<<MAX_SAFE_NOSEND*1000./rate;
                // }
                // else{
                //     auto i=receivingReportTimer.interval();
                //     receivingReportTimer.stop();
                //     receivingReportTimer.start(2*i);
                //     ninfo<<"重启接收超时定时器："<<receivingReportTimer.interval();
                // }
            }
            receivingWatchdog.stop();
            receivingWatchdog.start();
            {
            //条件回复Report
            if(lastReportElapsedTime.elapsed()>=MAX_REPORT_TIMEOUT || chunkId >= lastReportChunk+MAX_REPORT_OFFSET || header.type==(int)MessageType::RequestReport){
                // bool isRttAvailable = dmh.chunkId >= lastReportChunk+MAX_REPORT_OFFSET;
                //侦测丢包
                chunkid_t start = chunkId>REPORT_BATCH?chunkId-REPORT_BATCH:0;
                // // QList<chunkid_t> loss;
                // // for(chunkid_t i=start;i<chunkId;i++){
                // //     if(!receivingBuf.contains(i)){
                // //         loss.append(i);
                // //     }
                // // }
                // QList<QPair<chunkid_t,chunkid_t>> lossRangeList;
                // // loss.append(UINT_MAX);//用于输出最后一个区间
                // // if(!std::is_sorted(loss.begin(),loss.end())){std::sort(loss.begin(),loss.end());}
                // // for(chunkid_t i=0,last=0,start=0;i<loss.size();i++){
                // //     if(loss[i]-last>1){//生成连续闭合区间
                // //         lossRangeList.append(qMakePair(start,last));
                // //         start=loss[i];
                // //     }
                // //     last=loss[i];
                // // }
                // // if(!lossRangeList.empty() && lossRangeList[0]==qMakePair(0u,0u)){lossRangeList.pop_front();}//删除第一个[0,0]区间
                // // ninfo<<"Report"/*":loss="<<loss*/<<"lossRangeList"<<lossRangeList;
                // auto it=receivingBuf.upperBound(start);
                // if(it!=receivingBuf.begin()){
                //     --it;
                //     if(it.key()<start)start=it.key();
                // }
                // qint64 last=start-1;
                // for(;it!=receivingBuf.end();++it){
                //     if(it.key()-last>1){//区间不连续
                //         lossRangeList.append(qMakePair(last+1,it.key()-1));
                //     }
                //     last=it.key();
                // }
                //构造报文
                QByteArray body = generateLossRange(start,chunkId);
                ReportMessageHeader rmh;
                rmh.src=deviceId;
                rmh.type=(quint16)MessageType::Report;
                rmh.isRttAvailable=/*isRttAvailable*/true;
                rmh.start=start;
                rmh.isEmpty=body.isEmpty();
                rmh.lastReceive=chunkId;
                auto e=lastReportElapsedTime.nsecsElapsed()/1.e6;
                rmh.deliverRate= e!=0?delivered*1000./e:1;
                // if(rmh.deliverRate<1||rmh.deliverRate>10000){
                //     nwarning<<"DeliverRate="<<rmh.deliverRate<<" delivered="<<delivered<<" elapsed="<<e;
                // }
                // QByteArray msgBody;
                // // for(chunkid_t l:loss){
                // //     l=::qToBigEndian(l);
                // //     char lo[sizeof(l)];
                // //     memcpy(lo,&l,sizeof(l));
                // //     msgBody.append(lo,sizeof(l));
                // // }
                // msgBody.resize(2*lossRangeList.size()*sizeof(chunkid_t));
                // auto ptr = msgBody.data();
                // auto insertNum = [&](chunkid_t num){
                //     num=::qToBigEndian(num);
                //     // char src[sizeof(num)];
                //     // memcpy(src,&num,sizeof(num));
                //     // msgBody.append(src,sizeof(src));
                //     //快速插入
                //     memcpy(ptr,&num,sizeof(num));
                //     ptr+=sizeof(chunkid_t);
                // };
                // for(auto range:std::as_const(lossRangeList)){
                //     insertNum(range.first);
                //     insertNum(range.second);
                // }
                //发送
                send(getHeaderBytes(rmh)+body,1,header.src);
                lastReportElapsedTime.restart();
                delivered=0;
                lastReportChunk=chunkId;
            }
            }
            break;
        }
        case MessageType::Report:{
            if(state!=State::Transferring){
                ncritical<<"Unacceptable state '"<<(int)state<<"' to handle Report message";
            }
            //解析包
            ReportMessageHeader rmh=getHeaderStruct<ReportMessageHeader>(rawMsg);
            QList<chunkid_t> loss;
            if(!rmh.isEmpty){//只有头中标记有丢包才读取
                // QBuffer mbody;mbody.open(QBuffer::ReadWrite);
                // mbody.write(rawMsg.mid(sizeof(rmh)));
                // mbody.seek(0);
                loss.reserve(200);
                // const QByteArray &mbody = rawMsg.mid(sizeof(rmh));
                auto srcPtr = rawMsg.data()+sizeof(rmh);
                auto totalSize = rawMsg.size()-sizeof(rmh);
                auto srcEnd = srcPtr+totalSize;
                for(;srcPtr<srcEnd;srcPtr+=2*sizeof(chunkid_t)){
                    if(srcEnd-srcPtr<(ptrdiff_t)(2*sizeof(chunkid_t))){
                        break;
                    }
                    chunkid_t start,end;
                    memcpy(&start,srcPtr/*mbody.read(sizeof(chunkid_t)).constData()*/,sizeof(chunkid_t));
                    start=::qFromBigEndian(start);
                    memcpy(&end,srcPtr+sizeof(chunkid_t)/*mbody.read(sizeof(chunkid_t)).constData()*/,sizeof(chunkid_t));
                    end=::qFromBigEndian(end);
                    //快速插入
                    if(end<start){
                        ncritical<<"Bad end"<<end<<"<start"<<start;
                        continue;
                    }
                    auto size = loss.size();
                    loss.resize(loss.size()+end-start+1);
                    auto ptr = loss.data()+size;
                    for(chunkid_t i=start;i<=end;++i,++ptr){
                        // loss.append(i);
                        *ptr=i;
                    }
                }
            }
            emit reportReceived(rmh,loss);
            break;
        }
        case MessageType::External:
            emit externalReceived(msg,header.src);
            break;
        case MessageType::KeepAlive:
            break;
        }
#ifdef NNPYRO_PERFORMANCE_ANALYSIS
        double el = performanceTimer.nsecsElapsed()/1.e6;
        if(el>=0.5){
            nwarning<<"Performance Warning : onCommunicationReadyRead() finished in"<<el<<"ms.";
        }
#endif
    }
    // if(datagram_cnt>3){
    //     ndb<<"datagram_cnt="<<datagram_cnt;
    // }
}


void RpepEngine::onPrivateControlMessageReceived(QString key, QVariant value, devid_t src){
    ninfo<<"Control message received.key="<<key<<",value="<<value<<",src="<<getStringByDeviceId(src);
    static long long testSeq = -1;testSeq++;
    if(key=="___START_TRANSFER___"){
        if(state==State::Ready){
            ninfo<<"transfer accepted.";
            state=State::Receiving;
            acceptableSender=src;
            RUN_LATER_THIS(sendControl("___ACCEPT_TRANSFER___","",src););
            //初始化缓冲区
            // receivingBuf.resize(value.toUInt());
            receivingBuf.init(CHUNK_SIZE,value.toUInt());
        }
        else{
            RUN_LATER_THIS(sendControl("___REFUSE_TRANSFER___","state="+QString::number((int)state),src););
            ninfo<<"transfer refused.state="+QString::number((int)state);
        }
    }
    if(key=="___ACCEPT_TRANSFER___"){
        emit transferAccepted();
    }
    if(key=="___REFUSE_TRANSFER___"){
        emit transferRefused(value.toString());
    }
    if(key=="___FINISH_TRANSFER___"){
        if(state!=State::Receiving){
            ncritical<<"Unacceptable state '"<<(int)state<<"' to handle ___FINISH_TRANSFER___";
            return;
        }
        ndb<<"totalNum:"<<value.toUInt();
        auto a = testSeq;
        auto testSeq=a;
        receivingWatchdog.stop();
        receivingWatchdog.start();
        //侦测丢包
        // QByteArray msg;
        // for(chunkid_t i=0;i<value.toUInt();i++){
        //     if(!receivingBuf.contains(i)){
        //         chunkid_t be = ::qToBigEndian(i);
        //         char bin[sizeof(be)];
        //         memcpy(bin,&be,sizeof(be));
        //         msg.append(bin,sizeof(be));
        //     }
        // }
        //侦测丢包
        // QList<QPair<chunkid_t,chunkid_t>> lossRangeList;
        // chunkid_t start=0;
        // auto it=receivingBuf.begin();
        // qint64 last=(qint64)start-1;
        // for(;it!=receivingBuf.end();++it){  
        //     if(it.key()-last>1){//区间不连续
        //         lossRangeList.append(qMakePair(last+1,it.key()-1));
        //     }
        //     last=it.key();
        // }
        // if(!receivingBuf.empty() && receivingBuf.lastKey()!=value.toUInt()-1){//存在尾丢
        //     lossRangeList.append(qMakePair(receivingBuf.lastKey()+1,(qint64)value.toUInt()-1));
        // }
        // if(receivingBuf.empty()){
        //     lossRangeList.append(qMakePair(0,(qint64)value.toUInt()-1));
        // }
        // // if(receivingBuf.lastKey()!=value.toUInt()-1){lossRangeList.append(qMakePair(receivingBuf.lastKey(),value.toUInt()-1));}
        
        // // for(qint64 i=0,rangeStart=-1;i<value.toUInt();++i){
        // //     if(!receivingBuf.contains(i)){
        // //         if(rangeStart==-1){//区间内第一个丢包
        // //             rangeStart=i;
        // //         }
        // //     }
        // //     if((receivingBuf.contains(i) || i==value.toUInt()-1) && rangeStart!=-1){//区间丢包结束或区间完毕
        // //         lossRangeList.append(qMakePair(rangeStart,i-1));
        // //         rangeStart=-1;
        // //     }
        // // }
        // QByteArray msg;
        // // for(chunkid_t l:loss){
        // //     l=::qToBigEndian(l);
        // //     char lo[sizeof(l)];
        // //     memcpy(lo,&l,sizeof(l));
        // //     msgBody.append(lo,sizeof(l));
        // // }
        // msg.resize(2*lossRangeList.size()*sizeof(chunkid_t));
        // auto ptr = msg.data();
        // auto insertNum = [&](chunkid_t num){
        //     num=::qToBigEndian(num);
        //     // char src[sizeof(num)];
        //     // memcpy(src,&num,sizeof(num));
        //     // msgBody.append(src,sizeof(src));
        //     //快速插入
        //     memcpy(ptr,&num,sizeof(num));
        //     ptr+=sizeof(chunkid_t);
        // };
        // ndb<<"lossRangeList.empty"<<lossRangeList.empty()<<"seq="<<testSeq;
        // for(auto range:std::as_const(lossRangeList)){
        //     insertNum(range.first);
        //     insertNum(range.second);
        // }
        //发送消息
        QByteArray msg = generateLossRange(0,receivingBuf.size());
        if(!msg.isEmpty()){
            ndb<<"重传"<<testSeq;
            RUN_LATER_THIS(sendControl("___REQUEST_RESEND___",QVariant(msg),src););
        }
        else{
            ndb<<"释放"<<testSeq;
            //发送complete
            RUN_LATER_THIS(sendControl("___TRANSFER_COMPLETE___","",src););
            //合并消息，前面已经确保了消息完整性
            QByteArray data;
            /*for(auto c : std::as_const(receivingBuf)){
                data.append(c);
            }*/
            if(receivingBuf.size()!=value.toUInt()){
                ncritical<<"Too little chunks received.expected "<<value.toUInt()<<",actual "<<receivingBuf.size()<<"seq="<<testSeq;
            }
            // for(auto it=receivingBuf.begin();it!=receivingBuf.end();it++){
            //     // //断言检测
            //     // if(it.key()!=(qint64)value.toUInt()-1 &&  it->size()!=CHUNK_SIZE){
            //     //     ncritical<<"ERROR:Invalid chunk "<<it.key();
            //     // }
            //     // data+=it.value();
            //     if(!it->has_value()){
            //         ncritical<<"Invalid data";
            //         abortReceiving();
            //         receiverReset();
            //         return;
            //     }
            //     data+=it->value();
            // }
            // ndb<<"contains:"<<receivingBuf.contains(44824);
            // if(receivingBuf.contains(44824)){
            //     ndb<<"chunk 44824 real payload size:"<<receivingBuf[44824].size();
            // }
            // ndb<<data.size();
            emit dataReceived(receivingBuf.takeAwayWholeData().readAll(),src);
            //重置状态
            // state=State::Ready;
            // receivingBuf.clear();
            // lastReportElapsedTime.invalidate();
            // lastReportChunk=-1;
            // acceptableSender=0;
            // receivingWatchdog.stop();
            // receivingReportTimer.stop();
            // delivered=0;
            receiverReset();
            ndb<<"receiver "<<testSeq<<"reset.";
            //从队列中取出任务并执行
            if(!transferTaskQueue.isEmpty()){
                QMetaObject::invokeMethod(this,[=,this]{
                    FileByteArray fba;
                    fba.append(data);
                    transferData(std::move(fba),transferTaskQueue.dequeue());
                });
            }
            receivingWatchdog.stop();
        }
    }
    if(key=="___REQUEST_RESEND___"){
        //解析包
        QSet<chunkid_t> loss;
        QByteArray msg = value.toByteArray();
        ninfo<<"size:"<<msg.size();
        QBuffer buf(&msg);
        buf.open(QBuffer::ReadOnly);
        while(!buf.atEnd()){
            chunkid_t start,end;
            memcpy(&start,buf.read(sizeof(start)).constData(),sizeof(start));
            start = ::qFromBigEndian(start);
            memcpy(&end,buf.read(sizeof(end)).constData(),sizeof(end));
            end = ::qFromBigEndian(end);
            for(chunkid_t i=start;i<=end;i++){
                loss.insert(i);
            }
        }
        ninfo<<"请求重传："<<loss;
        emit retransferRequested(loss);
    }
    if(key=="___TRANSFER_COMPLETE___"){
        emit transferCompleted();
    }
    if(key=="___TASK___"){
        //设置队列
        if(!transferTaskQueue.empty()){
            
        }
        transferTaskQueue.clear();
        QByteArray msgbody = value.toByteArray();
        QBuffer buf(&msgbody);
        buf.open(QBuffer::ReadWrite);
        while(!buf.atEnd()){
            devid_t t;
            memcpy(&t,buf.read(sizeof(t)).constData(),sizeof(t));
            t=::qToBigEndian(t);
            transferTaskQueue.enqueue(t);
        }
    }
    if(key=="___ABORT_TRANSFER___"){
        if(state==State::Receiving && src==acceptableSender){
            // receivingBuf.clear();
            // lastReportElapsedTime.invalidate();
            // lastReportChunk=-1;
            // acceptableSender=0;
            // receivingWatchdog.stop();
            // receivingReportTimer.stop();
            // delivered=0;
            receiverReset();
        }
    }
    if(key=="___ABORT_RECEIVING___"){
        if(state==State::Transferring && src==transferDestination){
            // transferBuf.clear();
            // transferDestination=0;
            // transferWatchdog.stop();
            // transferTaskQueue.clear(); //**** 不清空！！！ ****
            senderReset();
            if(transferTaskQueue.empty()){
                transferBuf.clear();
            }
            else{
                QMetaObject::invokeMethod(this,[this]{transferPreloadedData(transferTaskQueue.dequeue());},Qt::QueuedConnection);
            }
        }
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
