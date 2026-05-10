#include "viewmodel.h"
#include <windows.h>

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
    connect(businesslogic,&BusinessLogic::messageChanged,this,[=](QString m){o_status=m;});
    connect(bl,&BusinessLogic::businessEventOccurred,this,&ViewModel::on_businessEventOccur);
    connect(bl,&BusinessLogic::sendInfoChanged,this,&ViewModel::on_sendInfoChange);
    connect(bl,&BusinessLogic::scheduleUpdated,this,&ViewModel::on_schedule_update);
    connect(bl,&BusinessLogic::remoteFileFolderUpdated,this,&ViewModel::on_remoteFileFolderUpdate);
    connect(bl,&BusinessLogic::deviceListUpdated,this,&ViewModel::on_deviceListUpdate);
    connect(bl,&BusinessLogic::rttTestResultUpdated,this,&ViewModel::on_rttTestResultUpdate);
}


void ViewModel::on_businessEventOccur(BusinessLogic::BusinessEvent event, QVariantMap map){
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
    case BusinessLogic::BusinessEvent::DisableNoticeStateUpdated:
        o_disableNoticeState=map["state"].toBool();
        break;
    case BusinessLogic::BusinessEvent::DescriptionUpdated:
        o_description=map["description"].toString();
        break;
    case BusinessLogic::BusinessEvent::AutoSyncEnableStateUpdated:
        o_autoSyncState=map["state"].toBool();
        break;
    case BusinessLogic::BusinessEvent::PageIndexUpdated:
        o_currentPageIndex=map["index"].toInt();
        break;
    case BusinessLogic::BusinessEvent::Ipv6UsageStateUpdated:
        o_ipv6UsageState=map["state"].toBool();
        break;
    case BusinessLogic::BusinessEvent::DestoryShutdownBlock:
        emit destoryShutdownBlock();
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
        break;
    case BusinessLogic::BusinessEvent::SendedSuccessfully:
        o_status="发送成功";
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
    }
}


void ViewModel::on_sendInfoChange(TransmissionEngine::SendInfo info){
    
}


void ViewModel::on_schedule_update(QByteArray schedule){
    o_scheduleBytes=schedule;
}


void ViewModel::on_remoteFileFolderUpdate(QString folder, QSet<QPair<bool, QString> > list){
    emit remoteFolderUpdated(folder,list);
}


void ViewModel::on_deviceListUpdate(QList<device> deviceList){
    o_clients=deviceList;
}


void ViewModel::on_rttTestResultUpdate(QList<QVariantMap> rttResult){
    
}
