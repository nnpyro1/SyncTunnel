#pragma once
#ifndef BUSINESSLOGIC_H
#define BUSINESSLOGIC_H

#include <QObject>
#include "modules/communication/communication.h"
#include "modules/storage/storage.h"
#include "modules/signalling/signalling.h"
#include "modules/transmissionengine/transmissionengine.h"
#include "modules/remotecontrol/remotecontrolengine.h"
#include "modules/rpepengine/rpepengine.h"
#include "core/services/schedule.h"
#include <QSettings>
#include <QProcess>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonDocument>

using ipport = Communication::ipport;
using device = Communication::device;



class BusinessLogic : public QObject
{
    Q_OBJECT
    [[maybe_unused]] int useless; 
public:
    explicit BusinessLogic(QObject *parent = nullptr);
    virtual ~BusinessLogic();
    
    enum skinType{
        //普通款
        Dark,
        Light,
        //限量款
        Silver,
        Golden,
    };
    enum class MessageBoxType{
        Information,
        Warning,
        Critical,
    };
    enum class BusinessEvent{
        //操作类（需要提供参数，详见调用处）
        StyleSheetUpdated,
        CurrentSkinIndexUpdated,
        RecordLogStateUpdated,
        SettingsUpdated,
        AutoSyncEnableStateUpdated,
        PageIndexUpdated,
        DestoryShutdownBlock,
        CurrentDirUpdated,
        FileUploadProgressUpdated,
        //状态更新/提示类（可选提供参数，详见调用处）
        PremiumUiUnauthorized,
        GettingPublicIp,
        PublicIpGetFailed,
        GettingDeviceList,
        LoadedSuccessfully,
        SendedSuccessfully,
        ProxyBanned,
        SkinCheckFailed,            //需要QString error参数
        UsernamePasswordTooShort,
        UploadingFirstLaunchInformation,
        UploadingFirstLaunchInformationFailed,//需要QString error参数
        TestingRTT,
        RTTTestSuccessfully,
        WaitingForResponse,
        DFHNDeviceNotFound,
        ConnectedSuccessfully,//需要QString ipport参数
        SignallingFailed,//需要QString error
        CurrentPathSetFailed,
        Debug,//调试用，可能需参数
        FileUploadingFinished,//可选QString error
        ErrorOccurred,//必选QString error，仅用于Result反馈
    };
    Q_ENUM(BusinessEvent);
    
    // struct Result{
    //     bool is_succeeded;
    //     QString errorMessage;
        
    //     Result():is_succeeded(true),errorMessage(QString()){}
    //     Result(QString errorMessage):is_succeeded(false),errorMessage(errorMessage){}
    // }; 
     private:typedef Result RSLT;public:
    
public://公有函数
    Q_INVOKABLE void init();                                        //初始化
    Q_INVOKABLE void destory();                                     //销毁对象
    
    // Q_INVOKABLE void send(QByteArray msg,bool e=1,int d=-1);        //自动加密msg并发送给所有client,e标识是否需要加密,d标识发给哪个客户端
    Q_INVOKABLE Result sendFile(QSet<devid_t> dst,QSet<QString> incremental_sync_set=QSet<QString>());//发送文件给所有客户端
//    Q_INVOKABLE bool restartDebug();                                //切换当前调试状态，返回切换过后的状态
    Q_INVOKABLE bool checkSkin(skinType skin);                      //检查skin是否可用
    
signals:
    void messageChanged(QString msg);                                               //当显示在右下角的信息改变，仅为了TransmissionEngine兼容使用，其余禁止使用
//    void tempMessageChanged(QString msg,int timeout=0);                             //当显示在左下角的信息改变
//    void messageBoxRequested(QString title,QString content,MessageBoxType type);    //请求弹出messageBox
//    void operateRequested(QString object,QString method,QVariant value=QVariant()); //请求
    void businessEventOccurred(BusinessLogic::BusinessEvent event,QVariantMap args=QVariantMap()); //当事件触发
    void sendInfoChanged(CongestionControl::CongestionControlInput ipt,CongestionControl::CongestionControlOutput opt);                        //发送砖头
    
    void scheduleUpdated(QByteArray schedule);                                      //日程更新
    void remoteFileFolderUpdated(QString folder,QSet<QPair<bool,QString>> list);    //远程目录改变
    void deviceListUpdated(Devices deviceList);                                     //设备列表改变
    void rttTestResultUpdated(QList<QVariantMap> rttResult);                        //测试RTT结果更新，每个QVariantMap需要有id,ip,port,rtt,delay
    void receivingProgressUpdated(quint32 received,quint32 total);                  //接收进度更新
    
public slots://以下是公有槽，需在外部联接
    void on_folder_change(QDir current_dir);
    void on_settings_saved(QString username_, QString pwd_, QString mqttServer_, 
                           int mqttPort_, QString githubUser_, QString githubPat_, 
                           QVariant skin_, bool recordLog_, bool disableNotice_, 
                           QString description_);                   //当设置保存
    void on_hangup();                                               //当文件挂起
    void on_download();                                             //文件挂起下载
    void on_sync_pat();                                             //同步Github PAT
    void on_shutdown_current(int id);                               //关闭选中
    void on_test_rtt();                                             //测试RTT
    BusinessLogic::RSLT on_request_file(int index);                 //请求文件
    /** @brief subdir输入|开头代表确定*/
    void on_copy_remote_file_operation_requested(QString subdir,int);//远程复制文件
    void on_add_schedule(Schedule *schedule);                       //增加日程
    BusinessLogic::RSLT on_remove_schedule(int index);              //删除日程
    void on_suspended();                                            //退后台
    void on_resumed();                                              //进入前台
    void on_hangup_to_dfhn();                                       //挂起到DFHN
    void on_download_from_dfhn();                                   //从DFHN上面下
    void on_restart_all();                                          //重启全部
    RemoteControlEngine *getRemoteControlEngine();                  //获取远程控制引擎指针用于初始化RemoteControlWidget
    void on_start_remote(int index);                                //开始远控
    void on_stop_remote();                                          //停止远控
    
    device getPublicIp();
    
    void on_debug([[maybe_unused]]QVariant dbgArgs={});             //调试
    
private slots://私有槽
    void on_readyRead(QByteArray msg);                              //原始消息接收
    void on_SPTP_readyRead(QByteArray msg);                         //SPTP协议收到大包
    void on_SPTP_ctrlMsg_received(TransmissionEngine::msg_ctrl);    //SPTP协议收到控制包
    //以上是废弃接口，以下是新版可用
    void onControlReceived(QString  key,QVariant value,devid_t src);//收到控制消息
    void onDataReceived(QByteArray data,devid_t src);               //收到大型数据
    
private://私有函数 
    QByteArray encode(const QByteArray &msg);                       //加密msg并返回密文
    QByteArray decode(const QByteArray &msg);                       //解密msg并返回解密后的值
    QStringList generateScheduleText();                             //生成schedule的文本
    QByteArray serSchedule();                                       //序列号Schedule
    void unserSchedule(QByteArray dat);                             //反序列化schedule
    bool stat();                                                    //统计用户
    
private:
    // Communication *m_communication;
    // Signalling *m_signalling;
    Storage *m_storage;
    // TransmissionEngine *m_transmissionengine;
    RemoteControlEngine *m_remotecontrolengine;
    RpepEngine *m_rpepengine;
    
private://私有变量
    device public_ip;
    QDir current_dir;
    Devices clients;
    QString user_name,pwd;
    QMap<int,QByteArray> currentFileMap;//当前正在传输的文件列表
    int currentFileTotal;//文件传输总数
    QTimer timer_fileResend;
    bool is_uploading = false;
    QByteArrayList chunks;//文件区块
    QTimer timer_is_uploading;
    QTimer timer_clear_currentFileMap;
    ipport mqtt_server;
    QString user_github_name;
    QString user_github_PAT;
    QProcess *process_proxy;//代理进程
    QProcess *process_proxy_ui;//代理UI进程
//    Ui::Dialog *ui_dialog_diviceList;
    bool is_accept_shutdown = false;
    QWidget *widget_savePower=nullptr;//省电模式窗口
    QTimer timer_savePower;//省电模式刷新定时器
    QTimer timer_savePower_finish;//省电模式结束定时器
    QSet<ipport> test_if_connected_set;
    const int SEND_MAX_DELAY = 500;//发送文件最大延迟
    const int SEND_MIN_DELAY = 1;//发送文件最小延迟
    int send_current_delay = SEND_MAX_DELAY - 10;//当前发送延迟
    int send_stable_count = 0;//最近一次稳定的数量
    QMap<ipport,int> send_ack_count;//每个客户端发的ack数量（会清零）
    int receive_lost_count = 0;//接收丢包计数
//    QElapsedTimer elapsed_lase_ack;//最后一次收到ack的时间
    int send_req_ack_loop = 5;//请求ACK的窗口
    QMap<ipport,int> send_lost_count;//每个客户端发送的lost包数量
    int receive_last_pack_index = -1;//上次收到的包的编号
    int receive_last_ack_total = -1;//上次收到的ack中包总数
    int send_lost_loop_count = 0;//丢包/不丢包计数，正为不丢包，负为丢包
    QMap<int,int> rtt_result;
    QElapsedTimer elapsed_rtt;
    skinType currentSkin = Dark;
    QTimer timer_keepAlive;
    int receive_last_ack_index = 0;
    int send_current_reqAck;//发送方本次req_ack的数字，用于防止lost包的备份延迟到达.
    device remote_device = {"",0};//正在远程控制的设备
    QTimer timer_remote_sending;//被远程控制的设备发送屏幕截图的定时器
    device remote_be_device = {"",0};//远程控制这台设备的设备
//    QByteArray remote_current_screen;//当前收到的屏幕截图
    QSet<int> send_current_fastresend_map;//快重传集合
    QUuid send_req_ack_uuid;
    QJsonObject json_settings;
    QString receive_last_uuid;
    QSet<QString> incremental_sync_set;//增量同步集合
    QList<Schedule*> schedule_list;
    int currentSendDst = -1;
    QByteArray lastMessage;
    QString currentReliableUuid;
    QString currentReliableMsg = "";
    QStringList sendTask;
    QString device_description;
    int device_flag;
    bool is_DFHN = false;
    bool is_first_launch = false;
    QTimer timer_refresh;
    bool is_autoSync = false;
    QTimer timer_autoSync;
    QMap<QString,QByteArray> fileHashMap;
    QDir syncFolder = QDir("files");
    QSettings settings;
    QSet<devid_t> lastSyncDst;
    bool use_ipv6 = false;
    QString log;
};

#ifndef BUSINESSLOGIC_H_2
#define BUSINESSLOGIC_H_2
static QFile *logFile;
void log(QtMsgType t, const QMessageLogContext &context, const QString &logstr);
static bool output_to_file = false;
Q_GLOBAL_STATIC(QMutex,logFileMutex);
#endif

Q_DECLARE_METATYPE(BusinessLogic::BusinessEvent)

#endif // BUSINESSLOGIC_H
