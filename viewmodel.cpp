#include "viewmodel.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QMetaEnum>

ViewModel::ViewModel(BusinessLogic *businesslogic, QObject *parent) : QObject(parent),bl(businesslogic){
    //联接信号槽
    /*
    void messageChanged(QString msg);                                               //当显示在右下角的信息改变，仅为了TransmissionEngine兼容使用，其余禁止使
    void businessEventOccurred(BusinessEvent event,QVariantMap args=QVariantMap()); //当事件触发
    void sendInfoChanged(TransmissionEngine::SendInfo info);                        //发送砖头
    
    void scheduleUpdated(QStringList schedule);                                     //日程更新
    void remoteFileFolderUpdated(QString folder,QSet<QPair<bool,QString>> list);    //远程目录改变
    void deviceListUpdated(QList<device> deviceList);                               //设备列表改变
    void rttTestResultUpdated(QList<QVariantMap> rttResult);   
    */
    connect(bl,&BusinessLogic::messageChanged,this,[=,this](QString m){o_status=m;ndb<<m;});
    connect(bl,&BusinessLogic::businessEventOccurred,this,&ViewModel::on_businessEventOccur);
    connect(bl,&BusinessLogic::sendInfoChanged,this,&ViewModel::on_sendInfoChange);
    connect(bl,&BusinessLogic::scheduleUpdated,this,&ViewModel::on_schedule_update);
    connect(bl,&BusinessLogic::remoteFileFolderUpdated,this,&ViewModel::on_remoteFileFolderUpdate);
    connect(bl,&BusinessLogic::deviceListUpdated,this,&ViewModel::on_deviceListUpdate);
    connect(bl,&BusinessLogic::rttTestResultUpdated,this,&ViewModel::on_rttTestResultUpdate);
    connect(bl,&BusinessLogic::receivingProgressUpdated,this,[this](quint32 r,quint32 t){o_status=QString("正在接收:%1/%2").arg(r).arg(t);});    
}


void ViewModel::sendFile(QSet<devid_t> dsts){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->sendFile(dsts,incremental_sync_set););
}

void ViewModel::on_folder_change(QDir dir){
    o_current_dir=dir;
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_folder_change(dir););
}

void ViewModel::on_settings_saved(QString username_, QString pwd_, QString mqttServer_, int mqttPort_, QString githubUser_, QString githubPat_, QVariant skin_, bool recordLog_, bool disableNotice_, QString description_, bool stat_){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_settings_saved(username_,pwd_,mqttServer_,mqttPort_,githubUser_,githubPat_,skin_,recordLog_,disableNotice_,description_,stat_););
}

void ViewModel::on_hangup(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_hangup(););
}

void ViewModel::on_download(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_download(););
}

void ViewModel::on_sync_pat(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_sync_pat(););
}

void ViewModel::on_shutdown_current(int id){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_shutdown_current(id););
}

void ViewModel::on_test_rtt(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_test_rtt(););
}

void ViewModel::on_request_file(int index){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_request_file(index););
}

void ViewModel::on_copy_remote_file_operation_requested(QString subdir, int i){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_copy_remote_file_operation_requested(subdir,i););
}

void ViewModel::on_add_schedule(Schedule *schedule){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_add_schedule(schedule););
}

void ViewModel::on_remove_schedule(int index){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_remove_schedule(index););
}

void ViewModel::on_suspended(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_suspended(););
}

void ViewModel::on_hangup_to_dfhn(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_hangup_to_dfhn(););
}

void ViewModel::on_download_from_dfhn(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_download_from_dfhn(););
}

void ViewModel::on_restart_all(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_restart_all(););
}

void ViewModel::on_start_remote(int index){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_start_remote(index););
}

void ViewModel::on_stop_remote(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_stop_remote(););
}


void ViewModel::on_stat_accepted(){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_stat_accepted(););
}


device ViewModel::getPublicIp(){
    device public_ip;
    QMetaObject::invokeMethod(bl,[=, this,&public_ip]{
        public_ip=bl->getPublicIp();
    },Qt::BlockingQueuedConnection);
    return public_ip;
}

void ViewModel::on_debug(QVariantMap args){
    RUN_IN_CUSTOM_THREAD_THIS(bl,bl->on_debug(args););
}


void ViewModel::on_businessEventOccur(BusinessLogic::BusinessEvent event, QVariantMap map){
    ninfo<<"Business Event Occurred. Event:"<<QMetaEnum::fromType<BusinessLogic::BusinessEvent>().valueToKey((int)event)<<" map:"<<map;
    switch (event) {
    //操作类
    case BusinessLogic::BusinessEvent::StyleSheetUpdated:
        o_styleSheet=map["stylesheet"].toString();
        break;
    case BusinessLogic::BusinessEvent::CurrentSkinIndexUpdated:
        o_currentSkinIndex=map["index"].toInt();
        break;
    case BusinessLogic::BusinessEvent::RecordLogStateUpdated:
        o_recordLogState=map["state"].toBool();
        break;
    case BusinessLogic::BusinessEvent::SettingsUpdated:
        o_user_name=map["username"].toString();
        o_pwd=map["password"].toString();
        o_description=map["description"].toString();
        o_ipv6UsageState=map["ipv6usage"].toBool();
        o_disableNoticeState=map["disablenotice"].toBool();
        break;
    case BusinessLogic::BusinessEvent::AutoSyncEnableStateUpdated:
        o_autoSyncState=map["state"].toBool();
        break;
    case BusinessLogic::BusinessEvent::PageIndexUpdated:
        o_currentPageIndex=map["index"].toInt();
        break;
    case BusinessLogic::BusinessEvent::DestoryShutdownBlock:
        emit destoryShutdownBlock();
        break;
    case BusinessLogic::BusinessEvent::CurrentDirUpdated:
        o_current_dir=QDir(map["value"].toString());
        break;
    case BusinessLogic::BusinessEvent::FileUploadProgressUpdated:
        o_status=QString("正在挂起。当前步骤:")+QMetaEnum::fromType<Storage::ProcessingState>().valueToKey(map["step"].toInt())+"进度:"+QString::number(map["progress"].toDouble()*100)+"%";
        break;
    case BusinessLogic::BusinessEvent::RequestStat:
        emit statRequested();
        break;
    //状态更新/提示类（可选提供参数，详见调用处）
    case BusinessLogic::BusinessEvent::PremiumUiUnauthorized:
        emit messageBoxRequested("限定UI","您无权使用限定UI！",BusinessLogic::MessageBoxType::Warning);
        break;
    case BusinessLogic::BusinessEvent::GettingPublicIp:
        o_status="正在获取公网IP……";
        break;
    case BusinessLogic::BusinessEvent::PublicIpGetFailed:
        emit messageBoxRequested("获取公网IP失败","获取公网IP失败，应用程序将自动重启。\n\n如果这个现象多次出现，请排查以下原因\n1.请在家用或防火墙不严格的公用网络中使用应用程序，禁止在移动数据环境下使用，否则失败概率高\n2.请确保网络已开启并正常连接连接\n\n如果问题没有解决，可以立即咨询开发者nnpyro2@outlook.com 我们愿意接受并解答任何问题和建议。只要你反馈，7个工作日内一定解决。\n\n单击确定重新启动应用程序。",BusinessLogic::MessageBoxType::Critical,true,[]{Utils::restart();});
        break;
    case BusinessLogic::BusinessEvent::GettingDeviceList:
        o_status="正在获取上线设备列表……";
        break;
    case BusinessLogic::BusinessEvent::LoadedSuccessfully:
        o_status="加载成功";
        QTimer::singleShot(100,[=]{
            RemoteControlEngine *eng;
            QMetaObject::invokeMethod(bl,[=, this,&eng]{eng=bl->getRemoteControlEngine();},Qt::BlockingQueuedConnection);
            emit remoteControlEngineUpdated(eng);
        });
        break;
    case BusinessLogic::BusinessEvent::SendedSuccessfully:
        emit transferFinished();
        o_status="传输结束";
        break;
    case BusinessLogic::BusinessEvent::ProxyBanned:
        emit messageBoxRequested("加速功能已禁用","(开发者小彩蛋)\n\nO(∩_∩)O 哈哈~，我知道你的小心思\n但是，网络加速功能 已 经 被 禁 用 了 ！\n不要妄想通过投机取巧的方式开启fastgithub,加速函数都被我删了,没用的。\n\n再见！",BusinessLogic::MessageBoxType::Information);
        o_status="加速功能已禁用";
        emit tempMessageChanged("不要投机取巧哦~",15000);
        break;
    case BusinessLogic::BusinessEvent::SkinCheckFailed:
        emit messageBoxRequested("限定UI检查失败","检查限定UI时出现了一个错误:\n"+map["error"].toString()+"\n所以您无法使用限定UI",BusinessLogic::MessageBoxType::Critical);
        o_status="检查限定UI失败";
        break;            
    case BusinessLogic::BusinessEvent::UsernamePasswordTooShort:
        emit messageBoxRequested("用户名密码过短","用户名密码至少要求8字符",BusinessLogic::MessageBoxType::Warning);
        break;
    case BusinessLogic::BusinessEvent::UploadingFirstLaunchInformation:
        o_status="正在上传……";
        break;
    case BusinessLogic::BusinessEvent::UploadingFirstLaunchInformationFailed:
        emit messageBoxRequested("上传失败","上传时出现了一个错误:\n"+map["error"].toString()+"\n但是这并不影响你使用软件的任何功能。单击确定继续使用软件。",BusinessLogic::MessageBoxType::Critical);
        break;
    case BusinessLogic::BusinessEvent::TestingRTT:
        o_status="正在测试RTT……";
        break;
    case BusinessLogic::BusinessEvent::RTTTestSuccessfully:
        o_status="RTT测试成功";
        break;
    case BusinessLogic::BusinessEvent::WaitingForResponse:
        o_status="正在等待发送端响应";
        break;
    case BusinessLogic::BusinessEvent::DFHNDeviceNotFound:
        emit messageBoxRequested("未找到DFHN设备","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN",BusinessLogic::MessageBoxType::Warning);
        break;
    case BusinessLogic::BusinessEvent::ConnectedSuccessfully:
        emit tempMessageChanged("成功与"+map["ipport"].toString()+"建立连接");
        break;//需要QString ipport参数
    case BusinessLogic::BusinessEvent::SignallingFailed:
        emit messageBoxRequested("信令失败","信令遇到错误：\n"+map["error"].toString(),BusinessLogic::MessageBoxType::Critical);
        break;
    case BusinessLogic::BusinessEvent::CurrentPathSetFailed:
        emit messageBoxRequested("设置工作目录失败","无法设置工作目录，软件功能将受限",BusinessLogic::MessageBoxType::Critical);
        break;
    case BusinessLogic::BusinessEvent::Debug:
        emit debugSignal(map);
        break;
    case BusinessLogic::BusinessEvent::FileUploadingFinished:
        if(map.contains("error")){
            emit messageBoxRequested("操作失败","您刚刚的文件挂起（取下挂起的文件）操作出现错误。\n\n错误详细信息与位置\n错误位置:"+map["error"].toString(),BusinessLogic::MessageBoxType::Critical);
            o_status="失败";
            break;
        }
        o_status="完成";
        break;
    case BusinessLogic::BusinessEvent::RpepTransferErrorOccurred:{
        //解析错误字符串
        QStringList errors = map["error"].toString().split("\n");
        QString errorString="???",suggestion;
        if(!errors.empty()){
            if(errors[0]=="StateCheck"){
                QString stateString;int s;
                if(errors.size()>=2){
                    s=errors[1].mid(6).toInt();
                    switch(s){
                    case 0:
                        stateString="未连接";
                        suggestion="尝试重启(按住Ctrl再点击右上角的叉可以彻底关闭)SyncTunnel，观察错误。尝试查阅https://synctunnel.fwh.is/docs/troubleshooting.html或直接通过nnpyro@outlook.com咨询开发者\n对您造成不便，十分抱歉QAQ";
                        break;
                    case 1:
                        stateString="正在连接";
                        suggestion="请等待连接完成。若一直卡在此状态，尝试彻底关闭此软件(按住Ctrl再点击右上角的叉)并重新启动或查阅https://synctunnel.fwh.is/docs/troubleshooting.html或通过nnpyro@outlook.com咨询开发者";
                        break;
                    case 2:
                        stateString="可用";
                        suggestion="软件发神经出问题了。尝试通过nnpyro@outlook.com咨询开发者";
                        break;
                    case 3:
                        stateString="正在传输";
                        suggestion="软件正在传输文件。您可以切换到“设备和信息”查看传输状态并等待传输完成。\n若当前软件并没有在传输，可以尝试关闭软件(按住Ctrl再点击右上角的叉)并重新打开";
                        break;
                    case 4:
                        stateString="正在接收";
                        suggestion="软件正在接收。请等待接收完成。\n若当前软件并没有在传输，可以尝试关闭软件(按住Ctrl再点击右上角的叉以彻底关闭)并重新打开";
                        break;
                    case 5:
                        stateString="外部独占";
                        suggestion="协议发送能力正在被外部（如远程控制）独占。请尝试重新启动(按住Ctrl再点击右上角的叉可以彻底关闭)软件或查看https://synctunnel.fwh.is/docs/troubleshooting.html";
                        break;
                    case 6:
                        stateString="出错";
                        suggestion="Oops!!!协议出错了！\n请先尝试重新启动软件(按住Ctrl再点击右上角的叉可以彻底关闭)，然后查阅https://synctunnel.fwh.is/docs/troubleshooting.html尝试解决问题。\n若您无法解决问题，请咨询nnpyro@outlook.com";
                        break;
                    default:
                        stateString=QString::number(s);
                        suggestion="尝试访问https://synctunnel.fwh.is/docs/troubleshooting.html解决问题";
                    }
                }
                errorString=QString()+"软件当前处于"+stateString+"的状态。只有在“可用”状态下允许传输文件";
            }
            if(errors[0]=="preloadData"){
                errorString="传输缓冲区已有数据";
                suggestion="请重新启动本软件(按住Ctrl再点击右上角的叉可以彻底关闭)，或访问https://synctunnel.fwh.is/docs/troubleshooting.html或咨询开发者";
            }
            if(errors[0]=="sendControl"){
                errorString="发送控制消息失败";
                suggestion="请重试刚才的操作。若不行，则访问https://synctunnel.fwh.is/docs/troubleshooting.html或咨询开发者nnpyro@outlook.com";
            }
            if(errors[0]=="startTransfer"){
                if(errors.size()>=2){
                    if(errors[1]=="refused"){
                        QString reason;
                        if(errors.size()>=3){
                            reason=errors[2];
                        }
                        errorString="传输被对方拒绝(拒绝原因:"+reason+") 详细信息请参见https://synctunnel.fwh.is/docs/troubleshooting.html";
                        suggestion="参见https://synctunnel.fwh.is/docs/troubleshooting.html";
                    }
                    if(errors[1]=="timeout"){
                        errorString="传输发起超时";
                        suggestion="尝试重试、重新启动应用程序(按住Ctrl再点击右上角的叉可以彻底关闭)或参见https://synctunnel.fwh.is/docs/troubleshooting.html";
                    }
                }
            }
        }
        
        emit messageBoxRequested("错误","Oh No!出错了!",BusinessLogic::MessageBoxType::Critical,false,nullptr,nullptr,
                                 "错误:\n"+errorString+"\n\n建议的解决方案:\n"+suggestion+"\n\n原始错误文本:"+errors.join("\n")+"\n\n\nPS:若遇到棘手的问题，请联系nnpyro@outlook.com或在https://synctunnel.fwh.is/docs/troubleshooting.html反馈，您的反馈十分珍贵"
        );
    }
        break;
    case BusinessLogic::BusinessEvent::TransferAborted:
        o_status="传输强制终止";
        break;
    case BusinessLogic::BusinessEvent::Preloading:
        o_status="正在预加密:"+map["i"].toString()+"/"+map["total"].toString();
         break;
    case BusinessLogic::BusinessEvent::PunchFailed:
        o_status="打洞失败";
        break;
    case BusinessLogic::BusinessEvent::StartTransferFailed:
        o_status="传输开始失败";
        break;
    case BusinessLogic::BusinessEvent::FinishTransferFailed:
        o_status="传输结束失败";        
        break;
    case BusinessLogic::BusinessEvent::FileReceived:
        o_status="接收成功";
        break;
    }
}


void ViewModel::on_sendInfoChange(CongestionControl::CongestionControlInput ipt,CongestionControl::CongestionControlOutput opt){
    emit sendInfoChanged(ipt,opt);
}


void ViewModel::on_schedule_update(QByteArray schedule){
    o_scheduleBytes=schedule;
}


void ViewModel::on_remoteFileFolderUpdate(QString folder, QSet<QPair<bool, QString> > list){
    emit remoteFolderUpdated(folder,list);
}


void ViewModel::on_deviceListUpdate(Devices deviceList){
    o_clients=deviceList;
}


void ViewModel::on_rttTestResultUpdate(QList<QVariantMap> rttResult){
    
}
