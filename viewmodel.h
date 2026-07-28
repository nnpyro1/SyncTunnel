#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <QObject>
#include <businesslogic.h>
#include <core/basic/observable.h>
#include <modules/rpepengine/congestioncontrol/congestioncontrol.h>

//Q_DECLARE_METATYPE区
Q_DECLARE_METATYPE(QSet<QString>)
Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QStringList)


class ViewModel : public QObject
{
    Q_OBJECT
public:
    explicit ViewModel(BusinessLogic *businesslogic,QObject *parent = nullptr);
    
public slots://以下是直接从BusinessLogic迁移的槽
    void sendFile(QSet<devid_t> dsts=QSet<devid_t>());
    void on_folder_change(QDir dir);
    void on_settings_saved(QString username_, QString pwd_, QString mqttServer_, 
                           int mqttPort_, QString githubUser_, QString githubPat_, 
                           QVariant skin_, bool recordLog_, bool disableNotice_, 
                           QString description_);                   //当设置保存
    void on_hangup();                                               //当文件挂起
    void on_download();                                             //文件挂起下载
    void on_sync_pat();                                             //同步Github PAT
    void on_shutdown_current(int id=0);                             //关闭选中
    void on_test_rtt();                                             //测试RTT
    void on_request_file(int index=0);                              //请求文件
    /** @brief subdir输入|开头代表确定*/
    void on_copy_remote_file_operation_requested(QString subdir,int);//远程复制文件
    void on_add_schedule(Schedule *schedule=0);                     //增加日程
    void on_remove_schedule(int index=0);                           //删除日程
    void on_suspended();                                            //退后台
    void on_hangup_to_dfhn();                                       //挂起到DFHN
    void on_download_from_dfhn();                                   //从DFHN上面下
    void on_restart_all();                                          //重启全部
    void on_start_remote(int index=0);                              //开始远程控制
    void on_stop_remote();
    
    device getPublicIp();                                           //从BL获取公网IP
    
    void on_debug(QVariantMap args={});                             //调试
    
    //后续
    // void openFile();                                                //打开文件
    // void openFolder();                                              //打开当前目录
    // void refresh();                                                 //同时刷新业务和UI
    // void shutdown();                                                //本机关机
    // void startCopyingRemoteFile();
    // void sendTestMessage();                                         //发送测试消息
    // void openDriveCrypto();                                         //打开DriveCrypto
    // void addFile();                                                 //请求用户添加文件
    // void addIncremental();                                          //将所选文件添加到可选同步列表
    // void removeIncremental();                                       //将所选文件删除到可选同步列表
    // void exit();                                                    //退出APP
    // void restart();                                                 //重启
    
signals:
    void tempMessageChanged(QString tmpMessage,int maxtime=5000);
    void messageBoxRequested(QString title,QString content,BusinessLogic::MessageBoxType type,bool doublebtn=false,std::function<void()> actionOnOk=nullptr,std::function<void()> actionOnCancel=nullptr);
    void destoryShutdownBlock();
    void remoteFolderUpdated(QString folder,QSet<QPair<bool,QString>> list);
    void sendInfoChanged(CongestionControl::CongestionControlInput ipt,CongestionControl::CongestionControlOutput opt);
    void debugSignal(QVariantMap args);
    void remoteControlEngineUpdated(RemoteControlEngine *eng);
    
public://公有Observable
    OBS(QString,status);
    OBS(QString,styleSheet);
    OBS(int,currentSkinIndex);
    OBS(bool,recordLogState);
    OBS(bool,disableNoticeState);
    OBS(QString,description);
    OBS(bool,autoSyncState);
    OBS(int,currentPageIndex);
    OBS(bool,ipv6UsageState);
    OBS(QByteArray,scheduleBytes);
    OBS(Devices,clients);
    OBS(QDir,current_dir);
    OBS(QString,user_name);
    OBS(QString,pwd);
//    OBS(QSet<QString>,incremental_sync_set);
//    QDir o_current_dir;
    QSet<QString> incremental_sync_set;
    
private slots://私有槽
    void on_businessEventOccur(BusinessLogic::BusinessEvent event,QVariantMap map);
    void on_sendInfoChange(CongestionControl::CongestionControlInput ipt,CongestionControl::CongestionControlOutput opt);
    void on_schedule_update(QByteArray schedule);
    void on_remoteFileFolderUpdate(QString folder,QSet<QPair<bool,QString>> list);
    void on_deviceListUpdate(Devices deviceList);
    void on_rttTestResultUpdate(QList<QVariantMap> rttResult);
    
private:
    BusinessLogic *bl;
};

#endif // VIEWMODEL_H
