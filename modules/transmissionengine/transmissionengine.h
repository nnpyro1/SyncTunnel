#ifndef TRANSMISSIONENGINE_H
#define TRANSMISSIONENGINE_H

#include <QObject>
#include <general.h>
#include <modules/communication/communication.h>
#include <qcachedbytearray.h>
#include <QUuid>
#include <QTimer>
#include <QSound>
#include <QQueue>
#include <memory>
#include <QMediaPlayer>
#include <core/basic/utils.h>

class TransmissionEngine : public QObject
{
    Q_OBJECT
public://构造/析构&枚举&结构体成员
    explicit TransmissionEngine(Communication * m_communication,QString username,QString pwd,device public_ip,QObject *parent = nullptr);//不提供默认构造函数，必需指定接管的communication因为需要预先STUN
    ~TransmissionEngine();
    enum sendState{
        ss_excellent,
        ss_good,
        ss_normal_better,
        ss_normal_worse,
        ss_bad,
        ss_worst,
    };
    enum msg_type{
        mt_bh               =0x42484842,
        mt_json             ='{',
        mt_filebody         =0x46424246,
        mt_ctrl             =0x43545443,
    };
    
    struct msg_common{
        ipport src;
        QByteArray msg;
    };
    struct msg_ctrl:public msg_common{
        QByteArray ctrl;
        QVariant value=QVariant();
    };
    struct SendInfo{
        int i,total,reqAckLoop;
        double delay;
    };

public://公有函数
    //辅助对接communication
    void send(QByteArray msg,bool e=1,int d=-1);            //自动加密msg并发送给所有client,e标识是否需要加密,d标识发给哪个客户端
    QByteArray encode(const QByteArray &msg);               //加密msg并返回密文
    QByteArray decode(const QByteArray &msg);               //解密msg并返回解密后的值
    //SPTP协议
    void SPTP_sendTo(int n,QByteArray data);          //自动分包并发送
    void SPTP_send(QByteArray msg,QList<device> dst);       //自动规划路径并发送给dst
    bool sendReliableMessage(int dst, QString msg);         //向dst发送可靠消息，阻塞直到对方收到
    QByteArray SPTP_sendCommon(QByteArray msg,int d=-1);    //发送普通二进制消息，返回消息。d输入-3代表不发
    QByteArray SPTP_sendCtrl(QByteArray ctrl,QVariant v=QVariant(),int d=-1);//发送控制消息。返回消息。d输入-3代表不发
    
    //其他函数
    void setClients(QList<device> clients);                 //设置设备列表
    QList<device> Clients();                                //获取设备列表
//    Communication *communicationObject();                   //线程安全地获取接管的communication对象       #####未实现。TransmissionEngine不线程安全
    void multiDelay(float ms);                              //高精度定时
    
signals:
    void messageChanged(QString message);                   //当需要在页面上显示消息的时候触发
    void sendInfoChanged(SendInfo info);                    //发送状态
    void communicationReadyRead(QByteArray msg);            //对于readyRead，应该联接这个而非Communication::readyRead
    void SPTP_readyRead(QByteArray data);                   //SPTP发送数据可读
    void SPTP_sendFinished();                               //消息发送结束，不管是否成功
    void reliableMessageReceived(QString msg);              //可靠消息收到。
    void SPTP_commonMsgReceived(msg_common msg);            //收到common消息（少见）
    void SPTP_ctrlMsgReceived(msg_ctrl msg);                //控制消息收到
    
signals://私有
    void signal_test_if_connected_finished(QPrivateSignal);         //连通性测试完成
    void signal_reqAck_finished(QPrivateSignal);                    //请求ack操作完成
    void signal_resend_finished(QPrivateSignal={});                 //对方重传完成
    void signal_reliableMessage_received(QString msg,QPrivateSignal={});//私有：可靠消息收到了
//    void signal_file_send_completed(QPrivateSignal={});             //文件发送成功并且状态成功清除 ### 和signal_resend_finished重复
    
private slots:
    void on_readyRead();
    void on_request_resend();
    void on_reliableMessage_received(QString msg,QPrivateSignal={});
    void on_bh_received(QByteArray msg);
    
private://私有定义
#pragma pack(push,1)
    struct header_filebody_p{
        qint32 check_type; //证明是header_filebody
        qint32 no;         //当前文件序号
        qint32 total;      //文件总数
    };
    struct header_common_p{
        qint32 check_type;
        quint8 ip_type;     //4=ipv4,6=ipv6
        quint64 src_public_ip_1;
        quint64 src_public_ip_2;
        quint16 src_port;
    };
    struct msg_ctrl_p{
        qint32 check_type;
        char ctrl[20];
        char value[1400];
    };
//    struct header_ctrl
#pragma pack(pop)
    struct file_sending_task{
        device dst;
        const QByteArray msg;
        file_sending_task(device dst_, QByteArray msg_) 
                : dst(dst_), msg(std::move(msg_)) {}
    };
    QVector<QVector<QPair<ipport,ipport>>> planAutoSend(QList<device> dsts);//自动规划向dsts发送的路径
    
private://私有对象/变量
    Communication *m_communication;//接管的Communication模块
    QString user_name,pwd;//用户凭证
    QList<device> clients;//用户列表
    device public_ip;
    int currentSendDst = -1;
    QSet<ipport> test_if_connected_set;
    bool is_uploading;//废弃接口
    QList<QByteArray> chunks;
    const int SEND_MAX_DELAY = 500;//发送文件最大延迟
    const int SEND_MIN_DELAY = 1;//发送文件最小延迟
    int send_current_delay = SEND_MAX_DELAY-10;
    int send_req_ack_loop = 5;//请求ACK的窗口
    QMap<ipport,int> send_lost_count;//每个客户端发送的lost包数量
    int receive_last_pack_index = -1;//上次收到的包的编号
    int receive_last_ack_total = -1;//上次收到的ack中包总数
    int send_lost_loop_count = 0;//丢包/不丢包计数，正为不丢包，负为丢包
    int send_current_reqAck;//发送方本次req_ack的数字，用于防止lost包的备份延迟到达.
    QUuid send_req_ack_uuid;
    QSet<int> send_current_fastresend_map;//快重传集合
    QTimer timer_keepAlive,timer_clear_currentFileMap,timer_is_uploading,timer_fileResend;
    QString currentReliableMsg;
    bool reliableMsg_available = false;
    QByteArray lastMessage;
    QMap<int,QByteArray> currentFileMap;//当前正在传输的文件列表
    int currentFileTotal;//文件传输总数
    QStringList sendTask;
    QString receive_last_uuid;
//    QString currentReliableUuid;
    device receive_sender;
    QSet<QString> processedReliableUuids; // 用于去重的UUID集合
    QMap<QString,QString> reliableMessages;
    QQueue<std::shared_ptr<const file_sending_task>> queue_fileSendingTask;
};

#endif // TRANSMISSIONENGINE_H
