#ifndef RPEPENGINE_H
#define RPEPENGINE_H

#include "general.h"
#include <QObject>
#include <modules/communication/communication.h>
#include <modules/rpepengine/congestioncontrol/congestioncontrol.h>
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
        Receiving,          //正在接收
        Busy,               //不可用，服务被其他对象占用
        Error               //错误
    };
    
    enum class Event{
        Error,//参数中有QString error，仅在讲状态设置为Error的时候使用
        GettingPublicIp,
        RegisteringOnline,
        Ready,//加载成功
        Punch,//打洞成功，加设备ID QString id
        TransferAborted,
    };
    
    enum class Error{
        PublicIp,           //公网ip失败
        Punch,              //打洞失败
        StartTransfer,      //无法开始
        FinishTransfer,     //无法结束
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
    Result sendControl(QString key, QVariant value, devid_t destination);                   //发送控制消息。外部控制消息禁止以___开头且结尾
    
    //属性/信息获取
    QString getUsername();
    QString getPassword();
    ipport getMqttBroker();
    Devices getAllDevices();                                                                //获取所有在线设备
    State getState();
    device getPublicIp();                                                                   //获取公有IP
    
    //外部接口
    bool acquireBusy();                                                                     //请求繁忙
    Result externalSend(QByteArray data,bool e=1,int d=1);                                  //外部调用send;
    void releaseBusy();                                                                     //释放繁忙
    
    //应急
    void reset();                                                                           //取消任何传输，清空所有状态，重置到Ready
    
signals:
    void dataReceived(QByteArray data,devid_t src);                                         //收到大数据包
    void controlReceived(QString key,QVariant value,devid_t src);                           //收到控制消息
    void eventOccurred(RpepEngine::Event event,QVariantMap args = QVariantMap());           //触发事件
    void deviceUpdated();                                                                   //Signalling转移：设备列表更新
    void deviceOnline(devid_t dev);                                                         //Signalling转移：设备上线
    void deviceOffline(devid_t dev);                                                        //Signalling转移：设备下线
    void externalReceived(QByteArray data,devid_t src);                                     //外部
    void congestionControlInfoUpdated(CongestionControl::CongestionControlInput ipt,CongestionControl::CongestionControlOutput opt);
    void receivingProgressUpdated(quint32 received,quint32 total);                          //接收进度更新
    void errorOccurred(RpepEngine::Error err);                                              //错误产生
    
private:
#pragma pack(push,1)
    using chunkid_t = quint32;
    struct CommonHeader{
        devid_t src;
        quint16 type;
        quint16 version=CURRENT_VERSION;
        quint16 reserved=0;
    };
    struct DataMessageHeader : public CommonHeader{
        chunkid_t chunkId;
        chunkid_t totalChunkNum;
    };
    struct ControlMessageHeader : public CommonHeader{
        char uuid[33];
        quint16 keySize;
    };
    struct ReportMessageHeader : public CommonHeader{
        bool isRttAvailable;
        bool isEmpty;
        chunkid_t start;
        chunkid_t lastReceive;
        double deliverRate;
    };

#pragma pack(pop)
    static CommonHeader qToBigEndian(CommonHeader h);
    static CommonHeader qFromBigEndian(CommonHeader h);
    static DataMessageHeader qToBigEndian(DataMessageHeader h);
    static DataMessageHeader qFromBigEndian(DataMessageHeader h);
    static ControlMessageHeader qToBigEndian(ControlMessageHeader h);
    static ControlMessageHeader qFromBigEndian(ControlMessageHeader h);
    static ReportMessageHeader qToBigEndian(ReportMessageHeader h);
    static ReportMessageHeader qFromBigEndian(ReportMessageHeader h);
    enum class MessageType{
        //联接状态/协议内部相关
        Punch               = 101,  //打洞
        KeepAlive           = 102,  //保活
        //可靠ControlMessage相关
        ReliableMessage     = 201,  //发布可靠消息
        ReliableResponse    = 202,  //回复201
        ReliableDone        = 203,  //回复202
        ReliableComplete    = 204,  //回复203
        //DataMessage传输相关
        DataPayload         = 301,  //数据载荷
        Report              = 302,  //丢包报告
        RequestReport       = 303,  //主动请求Report
        //外部
        External            = 401,  //外部使用
    };
    
    void send(QByteArray msg,bool e=1,int d=-1);
    QByteArray encode(const QByteArray &msg);
    QByteArray decode(const QByteArray &msg);
    Result punch(QSet<devid_t> dsts);
    template<typename T>static QByteArray getHeaderBytes(T header);
    template<typename T>static T getHeaderStruct(const QByteArray msg);
    Result transferData(QByteArray data,devid_t dst);
    Result preloadData(QByteArray data);                                                //预加载数据存储在transferBuf内
    Result transferPreloadedData(devid_t dst);                                          //使用成员变量transferBuf指定数据，要求transferBuf必需是已加密的完整数据包结构
    QVector<QVector<QPair<ipport,ipport>>> planAutoSend(Devices dsts);                  //自动规划向dsts发送的路径
    void abortReceiving();
    void receiverReset();                                                               //接收方重置桩台
    void senderReset();                                                                 //发送方重置状态
    
private://private signals
    Q_SIGNAL void punchReceived(devid_t sender,int seq);
    Q_SIGNAL void reliableStepsReceived(MessageType received,QString uuid);//received只允许是201~204
    Q_SIGNAL void transferAccepted();
    Q_SIGNAL void transferRefused(QString reason);
    Q_SIGNAL void reportReceived(ReportMessageHeader report,QList<chunkid_t> loss);
    Q_SIGNAL void retransferRequested(QSet<chunkid_t> loss);
    Q_SIGNAL void transferCompleted();
    Q_SIGNAL void transferAborted();
    
private slots:
    void onCommunicationReadyRead();
    void onPrivateControlMessageReceived(QString key, QVariant value, devid_t src);
    
private:
    Communication *m_communication;
    Signalling *m_signalling;
    
    QString username,pwd;
    ipport mqttBroker;
    ipport public_ip;
    devid_t deviceId;
    State state = RpepEngine::State::Invalid;
    Devices devices;
    QSet<devid_t> unconnectedDevices;//打洞失败的设备。
    QMap<QString,QPair<QString,QVariant>> pendingReliableMessages;
    //以下是发送方变量
    QList<QByteArray> transferBuf;
    devid_t transferDestination = 0;
    QTimer transferWatchdog;//发送方对接收方的看门狗，接收方超过指定时间没有发送Report就取消传输
    //以下是接收方变量
    QMap<chunkid_t,QByteArray> receivingBuf;//接收缓冲区
    QElapsedTimer lastReportElapsedTime;//上次回复Report的时间
    chunkid_t lastReportChunk;
    devid_t acceptableSender;//接收方可接受的发送方。仅在state=Receiving时允许非零
    QTimer receivingWatchdog;
    QTimer receivingReportTimer;
    chunkid_t delivered=0;
    //专有成员结束
    QQueue<devid_t> transferTaskQueue;
    QTimer timer_keepAlive;
    QByteArray preprocessedKey;//预处理过的密码
    
    static const int MAX_STUN_RETRIES = 3;
    static const int CURRENT_VERSION = 1;
    static const int MIN_COMPATIBLE_VERSION = 1;
    static const int MAX_RELIABLE_RETRIES = 8;
    static const int RELIABLE_INTERVAL = 1000;
    static const int CHUNK_SIZE = 1348;
    static const int MAX_TIMEOUT = 5000;
    static const int MAX_REPORT_TIMEOUT = 2000;
    static const int MAX_REPORT_OFFSET = 2;
    static const int REPORT_BATCH = 2800;
    static const int INITIAL_RATE = 10;
    static const int KEEPALIVE_INTERVAL = 15000;
    static const int TRANSFER_WATCHDOG_TIMEOUT = 20000;
    static const int RECEIVING_WATCHDOG_TIMEOUT = 20000;
    static const int MAX_SAFE_NOSEND = 4;
};

#endif // RPEPENGINE_H
