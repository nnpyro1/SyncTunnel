#ifndef RPEPENGINE_H
#define RPEPENGINE_H

#include "general.h"
#include <QObject>
#include <modules/communication/communication.h>
#include <modules/signalling/signalling.h>

class RpepEngine : public QObject
{
    Q_OBJECT
public:
    explicit RpepEngine(QObject *parent = nullptr);
    ~RpepEngine();
    
    enum class State{
        Invalid,            //无效。未init
        Connecting,         //正在连接
        Ready,              //可用
        Transferring,       //正在传输
        Busy,               //不可用，服务被其他对象占用
        Error               //错误
    };
    
    enum class Event{
        Error,//参数中有QString error，仅在讲状态设置为Error的时候使用
        GettingPublicIp,
        RegisteringOnline,
        Ready,//加载成功
        Punch,//打洞成功，加设备ID QString id
    };

public://公有接口
    void setUsername(QString username);
    void setPassword(QString pwd);
    void setMqttBroker(ipport broker);
    
    Result init();
    void destroy();
    
    //大数据包
    Result transfer(QByteArray data,QSet<devid_t> destinations);                            //开始分片传输
    void abortTransfer();                                                                   //强制终止传输
    
    //可靠控制消息
    Result sendControl(QByteArray key,QByteArray value,QSet<devid_t> destinations);         //发送控制消息。外部控制消息禁止以___开头
    
    //属性/信息获取
    QString getUsername();
    QString getPassword();
    ipport getMqttBroker();
    Devices getAllDevices();                                                                //获取所有在线设备
    State getState();
    
signals:
    void dataReceived(QByteArray data,devid_t src);                                         //收到大数据包
    void controlReceived(QByteArray key,QByteArray value,devid_t src);                      //收到控制消息
    void eventOccurred(RpepEngine::Event event,QVariantMap args = QVariantMap());           //触发事件
    void deviceUpdated();                                                                   //Signalling转移：设备列表更新
    void deviceOnline(devid_t dev);                                                         //Signalling转移：设备上线
    void deviceOffline(devid_t dev);                                                        //Signalling转移：设备下线
    
private:
#pragma pack(push,1)
    struct CommonHeader{
        devid_t src;
        quint16 type;
        quint16 version=1;
        quint16 reserved=0;
    };
    struct DataMessageHeader : public CommonHeader{
        quint32 chunkId;
        quint32 totalChunkNum;
    };
#pragma pack(pop)
    CommonHeader qToBigEndian(CommonHeader h);
    CommonHeader qFromBigEndian(CommonHeader h);
    DataMessageHeader qToBigEndian(DataMessageHeader h);
    DataMessageHeader qFromBigEndian(DataMessageHeader h);
    enum class MessageType{
        //联接相关
        Punch               = 101,  //打洞
        //可靠ControlMessage相关
        ReliableMessage     = 201,  //发布可靠消息
        ReliableResponse    = 202,  //回复201
        RelialeDone         = 203,  //回复202
        ReliableComplete    = 204,  //回复203
    };
    
    void send(QByteArray msg,bool e=1,int d=-1);
    QByteArray encode(const QByteArray &msg);
    QByteArray decode(const QByteArray &msg);
    Result punch(QSet<devid_t> dsts);
    template<typename T>QByteArray getHeaderBytes(T header);
    template<typename T>T getHeaderStruct(const QByteArray msg);
    
    
private://private signals
    Q_SIGNAL void punchReceived(devid_t sender,int seq);
    
private slots:
    void onCommunicationReadyRead();
    
private:
    Communication *m_communication;
    Signalling *m_signalling;
    
    QString username,pwd;
    ipport mqttBroker;
    ipport public_ip;
    State state = RpepEngine::State::Invalid;
    Devices devices;
    QSet<devid_t> unconnectedDevices;//打洞失败的设备。
    QMap<QString,QByteArray> pendingReliableMessages;
    
    const int MAX_STUN_RETRIES = 3;
    const int CURRENT_VERSION = 1;
    const int MIN_COMPATIBLE_VERSION = 1;
};

#endif // RPEPENGINE_H
