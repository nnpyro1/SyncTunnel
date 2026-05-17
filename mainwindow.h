#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QDir>
//#include <QMap>
//#include <QTimer>
//#include <QDropEvent>
//#include "modules/communication/communication.h"
//#include "modules/storage/storage.h"
//#include "modules/signalling/signalling.h"
//#include "modules/transmissionengine/transmissionengine.h"
#include "dialogs/dialog_remotefile.h"
#include <QCloseEvent>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
//#include <QProcess>
//#include <QElapsedTimer>
#include <QSystemTrayIcon>
//#include <QtCharts>
#include "dialogs/dialog_schedule.h"
//#include <qcachedbytearray.h>
#include <dialogs/dialog_selectsyncdst.h>
#include <dialogs/dialog_help.h>
#include <general.h>
#include <dialogs/wizard_startup.h>
//#include <QResizeEvent>
//#include <functional>
//#include <businesslogic.h>
#include <viewmodel.h>
#include <QShortcut>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
//    enum skinType{
//        //普通款
//        Dark,
//        Light,
//        //限量款
//        Silver,
//        Golden,
//    };
//    enum sendState{
//        ss_excellent,
//        ss_good,
//        ss_normal_better,
//        ss_normal_worse,
//        ss_bad,
//        ss_worst,
//    };
//    enum msg_type{
//        mt_json             ='{',
//        mt_filebody         =0x46424246,
//    };
//    enum attack_type{
//        at_possible,
//        at_minor,
//        at_influential,
//        at_destructive,
//    };
//    enum language{
//        language_chinese,
//        language_english,
//    };

////    Q_ENUM(attack_type)

//#pragma pack(push,1)
//    struct header_filebody{
//        qint32 check_type; //证明是header_filebody
//        qint32 no;         //当前文件序号
//        qint32 total;      //文件总数
//    };
//#pragma pack(pop)
    
////    typedef void (*fpvoid)(); 
//    using fpvoid = std::function<void()>;
    
public://公有函数
    MainWindow(ViewModel *vm,QWidget *parent = nullptr,std::function<void(QString)> func_update = nullptr,bool bShow=false);
    ~MainWindow();
    
    void show_dir();                            //显示目录到Widget上
//    void send(QByteArray msg,bool e=1,int d=-1);//自动加密msg并发送给所有client,e标识是否需要加密,d标识发给哪个客户端
//    QByteArray encode(QByteArray msg);          //加密msg并返回密文
//    QByteArray decode(QByteArray msg);          //解密msg并返回解密后的值
//    QByteArray mergeFile(QDir folder,bool c=1); //合并文件
//    Q_INVOKABLE void sendFileTo(int n);         //自动分包并发送
    Q_INVOKABLE void sendFile(QList<device> dst = QList<device>());//发送文件给所有客户端
//    void releaseFile(QByteArray msg);           //释放文件
//    void savePower();                           //省电模式
//    bool checkSkin(skinType skin);              //检查skin是否可用
//    void copy_remote_file(int index);           //远程操作文件
//    void multiDelay(float ms);                  //高精度定时
//    QByteArray readLine(QByteArray &ba);        //读取一行并删除这一行，返回不带换行符的消息
//    void attackProtection(attack_type type,     //攻击防护 攻击类型
//                          QString reason,       //攻击防护 原因
//                          QString solution,     //攻击防护 解决办法的描述
//                          fpvoid solution_fp,   //攻击防护 解决办法lambda
//                          fpvoid rec=0);        //攻击防护 恢复办法
    void restartDebug();                        //重新启用/禁用调试
    void hideTab(QTabWidget *tab,int index);    //隐藏tab中第index标签
//    bool sendReliableMessage(int dst, QString msg);//向dst发送可靠消息，阻塞直到对方收到
//    QVector<QVector<QPair<ipport,ipport>>> planAutoSend(QList<device> dsts);//自动规划向dsts发送的路径
//    QMap<QString,QByteArray> generateFileHashMap(QDir baseDir);//更新文件哈希表
//    void initNetwork(std::function<void(QString)> a=nullptr);  //初始化网络
//    void restartNetwork();                      //重启网络
//    QList<QPair<QFileInfo,QDir>>traverseFolder(QDir dir);//遍历  
    
private slots://槽函数
    void on_folder_change();                    //当前目录改变（双击打开）
    void on_rightclick();                       //目录被右击
//    void on_readyRead(QByteArray msg);          //communication模块接受到新的消息
//    void on_SPTP_readyRead(QByteArray msg);     //SPTP协议收到大消息
//    void on_request_resend();                   //请求重传文件
    void on_settings_saved();                   //设置的保存按钮被点击
//    void on_hangup();                           //当文件挂起
//    void on_download();                         //当文件下载
//    void on_proxy();                            //当开始/停止网络加速
    void on_rightclick_deviceList();            //设备列表被右击
    void on_test_rtt();                         //当测试RTT
//    void on_SPTP_ctrlMsg_received(TransmissionEngine::msg_ctrl);//当SPTP收到控制消息 新版协议，以后不用Json了，迁移至此处
//    void restart();                             //跨平台重启
    void on_sendInfo_updated(TransmissionEngine::SendInfo info);
    
    void on_pushButton_debug1_clicked();//当调试
    
protected://继承
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    
    
signals://信号
//    void signal_test_if_connected_finished(QPrivateSignal);        //连通性测试完成
//    void signal_reqAck_finished(QPrivateSignal);                   //请求ack操作完成
    
private://模块
//    Communication *m_communication;
////    Request *request;
//    Storage *m_storage;
//    Signalling *m_signalling;
//    TransmissionEngine *m_transmissionengine;
    
    ViewModel *vm;
    
private://私有变量
    Ui::MainWindow *ui;
//    device public_ip;
//    QDir current_dir;
//    QList<Communication::device> clients;
//    QString user_name,pwd;
//    QMap<int,QByteArray> currentFileMap;//当前正在传输的文件列表
//    int currentFileTotal;//文件传输总数
//    QTimer timer_fileResend;
//    bool is_uploading = false;
//    QByteArrayList chunks;//文件区块
//    QTimer timer_is_uploading;
    QLabel *label_status;
//    QTimer timer_clear_currentFileMap;
//    ipport mqtt_server;
//    QString user_github_name;
//    QString user_github_PAT;
//    QProcess *process_proxy;//代理进程
//    QProcess *process_proxy_ui;//代理UI进程
////    Ui::Dialog *ui_dialog_diviceList;
//    bool is_accept_shutdown = false;
//    QWidget *widget_savePower=nullptr;//省电模式窗口
//    QTimer timer_savePower;//省电模式刷新定时器
//    QTimer timer_savePower_finish;//省电模式结束定时器
//    QSet<ipport> test_if_connected_set;
//    const int SEND_MAX_DELAY = 500;//发送文件最大延迟
//    const int SEND_MIN_DELAY = 1;//发送文件最小延迟
//    int send_current_delay = SEND_MAX_DELAY - 10;//当前发送延迟
//    int send_stable_count = 0;//最近一次稳定的数量
//    QMap<ipport,int> send_ack_count;//每个客户端发的ack数量（会清零）
//    int receive_lost_count = 0;//接收丢包计数
////    QElapsedTimer elapsed_lase_ack;//最后一次收到ack的时间
//    int send_req_ack_loop = 5;//请求ACK的窗口
//    QMap<ipport,int> send_lost_count;//每个客户端发送的lost包数量
//    int receive_last_pack_index = -1;//上次收到的包的编号
//    int receive_last_ack_total = -1;//上次收到的ack中包总数
//    int send_lost_loop_count = 0;//丢包/不丢包计数，正为不丢包，负为丢包
//    QMap<int,int> rtt_result;
//    QElapsedTimer elapsed_rtt;
//    skinType currentSkin = Dark;
//    QSystemTrayIcon *trayIcon = new QSystemTrayIcon;
//    QTimer timer_keepAlive;
//    Dialog_remoteFile *dialog_remoteFile;
//    int receive_last_ack_index = 0;
//    int send_current_reqAck;//发送方本次req_ack的数字，用于防止lost包的备份延迟到达.
//    device remote_device = {"",0};//正在远程控制的设备
//    QTimer timer_remote_sending;//被远程控制的设备发送屏幕截图的定时器
//    device remote_be_device = {"",0};//远程控制这台设备的设备
////    QByteArray remote_current_screen;//当前收到的屏幕截图
//    QSet<int> send_current_fastresend_map;//快重传集合
//    QChart *chart_send = new QChart;
//    QLineSeries *line_speed;
//    QLineSeries *line_ackloop;
//    QLineSeries *line_delay;
//    QValueAxis *axis_x = new QValueAxis;
//    QValueAxis *axis_y = new QValueAxis,*axis_y_r = new QValueAxis;
    QShortcut *shortcut_debug = new QShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_D),this);
//    QUuid send_req_ack_uuid;
//    QJsonObject json_settings;
//    QString receive_last_uuid;
//    QSet<QString> incremental_sync_set;//增量同步集合
//    language current_language = language_chinese;
//    QTranslator *translator = nullptr;
    Dialog_schedule *dialog_schedule;
    QList<Schedule*> schedule_list;
//    int currentSendDst = -1;
//    QByteArray lastMessage;
//    QString currentReliableUuid;
//    QString currentReliableMsg = "";
//    QStringList sendTask;
//    QString device_description;
//    int device_flag;
//    bool is_DFHN = false;
//    bool is_first_launch = false;
    QTimer timer_refresh;
//    bool is_autoSync = false;
//    QTimer timer_autoSync;
//    QMap<QString,QByteArray> fileHashMap;
//    QDir syncFolder = QDir("files");
//    QSettings settings;
//    QList<device> lastSyncDst;
#ifdef QT_DEBUG
    bool is_debug = true;
#else
    bool is_debug = false;
#endif
    
private:
    QSystemTrayIcon *trayIcon;
};


#endif // MAINWINDOW_H
