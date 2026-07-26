#include "businesslogic.h"
#include "qmetaobject.h"
#include "qnetworkreply.h"
#include <QDir>
#include <core/basic/utils.h>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_ANDROID
#include <QJniObject>
// #include <QAndroidApplication>
#endif
#include <../../libary/Qt-AES/qaesencryption.h>
#include <QApplication>
#include <passport.h>
#include <iostream>
#include <QRandomGenerator>
#include <QBuffer>
#include <modules/remotecontrol/remotecontrolwidget.h>


BusinessLogic::BusinessLogic(QObject *parent) : QObject(parent) ,useless(QDir().mkpath("config")) ,settings("config/settings.ini",QSettings::IniFormat){
    
}


BusinessLogic::~BusinessLogic(){
    
}


void BusinessLogic::init(){
    //对象创建
    // m_communication = new Communication;
    // m_signalling = new Signalling;
    m_storage = new Storage;
    // m_transmissionengine = nullptr;
    process_proxy = new QProcess(this);
    process_proxy_ui = new QProcess(this);
    m_rpepengine = new RpepEngine();
//    dialog_remoteFile = new Dialog_remoteFile(this);
    logFile = new QFile(QString("logs/%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")).toStdString().c_str(),this);
    
    //安卓平台设置
#ifdef Q_OS_ANDROID
    /*QString currentWorkPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    ninfo<<"工作目录信息配置正确。"<<currentWorkPath;
    if(!currentWorkPath.contains("Android/data")){
        ninfo<<"工作目录:"<<currentWorkPath;
        QString ph = "/storage/emulated/0/Android/data/com.nnpyro.SyncTunnel/";
//        ui->statusBar->showMessage("成功配置了工作目录",1);
        QDir dir(ph);
        if(!dir.exists())dir.mkpath(".");
        QDir::setCurrent(ph);
    }*/
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if(!context.isValid()){
        ncritical<<"context 无效";
        // QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\ncontext无效"));
        emit businessEventOccurred(BusinessEvent::CurrentPathSetFailed);
    }
    else{
        QJniObject fileObj = context.callObjectMethod("getExternalFilesDir","(Ljava/lang/String;)Ljava/io/File;",nullptr);
        if(!fileObj.isValid()){
            ncritical<<"fileObj 无效";
            // QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\nfileObj无效"));
            emit businessEventOccurred(BusinessEvent::CurrentPathSetFailed);
        }
        else{
            QJniObject o = fileObj.callObjectMethod("getAbsolutePath","()Ljava/lang/String;");
            ninfo<<"path:"<<o.toString();
            QString path = o.toString();
            if(path.isEmpty()){
                ncritical<<"path 无效";
                // QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\npath无效"));
                emit businessEventOccurred(BusinessEvent::CurrentPathSetFailed);
            }
            else{
                QDir(path).mkpath(".");
                QDir::setCurrent(path);
                ninfo<<"当前目录："<<QDir::currentPath();
            }
        }
    }
#endif
    
    //基本设置
    timer_is_uploading.setSingleShot(true);
    timer_clear_currentFileMap.setSingleShot(true);
#ifdef Q_OS_WIN
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);//设置优先级为高
#endif
//    ui->tableWidget_deviceList.
    //隐藏不必要的标签页
//    restartDebug();
#ifdef NNPYRO_USE_CONSOLE//使用控制台
    if(!QApplication::arguments().contains("CON_MODE")){if(!QDir("tools/").exists())QDir().mkpath("tools/");QFile::copy(":/rc/bin/Alacritty.exe","tools/Alacritty.exe");QProcess::startDetached("tools/Alacritty.exe",QStringList()<<"-e"<<QApplication::applicationFilePath()<<QApplication::arguments()<<"CON_MODE");close();QApplication::quit();}
#endif
#ifdef NNPYRO_COLORFULCON//使用控制台
    qInstallMessageHandler(static_cast<QtMessageHandler>(log));
#endif
    
    //先初始化目录
    if(!QDir("config").exists()){
        QDir("config").mkdir(".");
    }
    if(!QDir("files").exists()){
        QDir("files").mkdir(".");
    }
    //读取文件中的用户名密码
    QFile file1("config/1.nprivate0");
    file1.open(QIODevice::ReadOnly);
    QTextStream user_config_stream1(&file1);
    user_name = QByteArray::fromBase64(user_config_stream1.readLine().toUtf8());
    auto encryptedPwd=(QByteArray::fromBase64(user_config_stream1.readLine().toUtf8()));
    auto iv = encryptedPwd.mid(0,16);
    pwd = QAESEncryption::RemovePadding(QAESEncryption::Decrypt(QAESEncryption::AES_256,QAESEncryption::CBC,encryptedPwd.mid(16),SYNCTUNNEL_CRPT_KEY,iv));
    if(user_name.isEmpty())user_name="DefaultUser";
    //读取文件中的mqtt服务器设置
    QFile file2("config/2.nprivate0");
    file2.open(QIODevice::ReadOnly);
    QTextStream user_config_stream2(&file2);
    mqtt_server = ipport{
            QString(QByteArray::fromBase64(user_config_stream2.readLine().toUtf8())),
            static_cast<quint16>(user_config_stream2.readLine().toInt())
    };
    if(mqtt_server.ip.isEmpty()){
        mqtt_server={"broker.emqx.io",1883};
    }
    //读取文件中的Github服务器设置
    QFile file3("config/3.nprivate0");
    file3.open(QIODevice::ReadOnly);
    QTextStream user_config_stream3(&file3);
    user_github_name = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
    user_github_PAT = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
    //读取文件中应用程序配置
    QFile file4("config/4.nprivate0");
    file4.open(QIODevice::ReadOnly);
    QTextStream user_config_stream4(&file4);
    skinType skin = (skinType)user_config_stream4.readLine().toInt();
    QFile file_QSS;
//    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    switch(skin){
    case Dark:
        currentSkin=skin;
        file_QSS.setFileName(":/rc/style/dark.qss");
        file_QSS.open(QFile::ReadOnly);
//        setStyleSheet("");
//        qApp->setStyleSheet(file_QSS.readAll());
//        emit operateRequested("qApp","setStyleSheet",file_QSS.readAll());
        emit businessEventOccurred(BusinessEvent::StyleSheetUpdated,{{"stylesheet",file_QSS.readAll()}});
        break;
    case Light:
        currentSkin=skin;
        file_QSS.setFileName(":/rc/style/light.qss");
        file_QSS.open(QFile::ReadOnly);
//        setStyleSheet("");
//        qApp->setStyleSheet(file_QSS.readAll());
//        emit operateRequested("qApp","setStyleSheet",file_QSS.readAll());
        emit businessEventOccurred(BusinessEvent::StyleSheetUpdated,{{"stylesheet",file_QSS.readAll()}});
        break;
    case Golden:
        if(checkSkin(Golden)){
            currentSkin=skin;
            file_QSS.setFileName(":/rc/style/golden.qss");
            file_QSS.open(QFile::ReadOnly);
//            setStyleSheet("");
//            emit operateRequested("","setStyleSheet","");
////            qApp->setStyleSheet(file_QSS.readAll());
//            emit operateRequested("qApp","setStyleSheet",file_QSS.readAll());
//            emit operateRequested("","setWindowTitle",tr("SyncTunnel 同步隧道（金色流光限定版） 感谢您本对软件做出的贡献"));
            emit businessEventOccurred(BusinessEvent::StyleSheetUpdated,{{"stylesheet",file_QSS.readAll()}});
        }
        else{
//            emit messageBoxRequested(tr("SyncTunnel"),tr("您无法使用限量版UI。"),MessageBoxType::Information);
            emit businessEventOccurred(BusinessEvent::PremiumUiUnauthorized);
        }
        break;
    case Silver:
        if(checkSkin(Silver)){
            currentSkin=skin;
            file_QSS.setFileName(":/rc/style/silver.qss");
            file_QSS.open(QFile::ReadOnly);
//            setStyleSheet("");
//            emit operateRequested("","setStyleSheet","");
//            qApp->setStyleSheet(file_QSS.readAll());
//            emit operateRequested("","setWindowTitle",tr("SyncTunnel 同步隧道（银色星辰限定版）"));
            emit businessEventOccurred(BusinessEvent::StyleSheetUpdated,{{"stylesheet",file_QSS.readAll()}});
        }
        else{
//            emit messageBoxRequested(tr("SyncTunnel"),tr("您无法使用限量版UI。"),MessageBoxType::Information);
            emit businessEventOccurred(BusinessEvent::PremiumUiUnauthorized);
        }
        break;
    }
//    ui->comboBox_settings_uiskin->setCurrentIndex(currentSkin);
//    emit operateRequested("ui->comboBox_settings_uiskin","setCurrentIndex",currentSkin);
    emit businessEventOccurred(BusinessEvent::CurrentSkinIndexUpdated,{{"index",(int)currentSkin}});
    ninfo<<"current Skin="<<currentSkin;
//    读取JSON配置
    QFile file_json("config/config.json");
    file_json.open(QFile::ReadOnly);
    json_settings = QJsonDocument::fromJson(file_json.readAll()).object();
    file_json.close();
    if(json_settings["use_log"].toBool()){
        if(!QDir("logs").exists())QDir("logs").mkpath(".");
        logFile->open(QFile::WriteOnly);
        output_to_file = true;
        qInstallMessageHandler(static_cast<QtMessageHandler>(log));
        ninfo<<"日志输出重定向到文件";
//        ui->checkBox_settings_recordLog->setCheckState(Qt::Checked);
//        emit operateRequested("ui->checkBox_settings_recordLog","setCheckState",Qt::Checked);
        emit businessEventOccurred(BusinessEvent::RecordLogStateUpdated,{{"state",true}});
    }
//    if(json_settings["disable_notice"].toBool())/*ui->checkBox_settings_disableNotice->setCheckState(Qt::Checked);*/emit operateRequested("ui->checkBox_settings_disableNotice","setCheckState",Qt::Checked);
    if(json_settings["disable_notice"].toBool()) emit businessEventOccurred(BusinessEvent::RecordLogStateUpdated,{{"state",true}});
    device_description=json_settings["description"].toString();
//    device_flag=json_settings["device_flag"].toInt();
//    ui->lineEdit_settings_description->setText(device_description);
//    emit operateRequested("ui->lineEdit_settings_description","setText",device_description);
    //读取QSettings中ini配置
    //ini配置中包含几个节:ApplicationSettings(应用程序需要的本地设置,网络无关),NetworkSettings(网络相关)和UserSettings(用户凭证)
    if(settings.contains("ApplicationSettings/syncSourceDir")){
        syncFolder=QDir(settings.value("ApplicationSettings/syncSourceDir").toString());
    }
    else{
        syncFolder=QDir("files/");
    }
    is_autoSync=settings.value("ApplicationSettings/isAutoSync").toBool();
    
//    ui->checkBox_file_autoSync->setCheckState(is_autoSync?Qt::Checked:Qt::Unchecked);
//    emit operateRequested("ui->ui->checkBox_file_autoSync","setCheckState",is_autoSync?Qt::Checked:Qt::Unchecked);
    emit businessEventOccurred(BusinessEvent::AutoSyncEnableStateUpdated,{{"state",false}});
    //读取Schedule日程配置
    QFile file_schedule("config/schedule.dat");
    file_schedule.open(QFile::ReadOnly);
//    QDataStream d(&file_schedule);
//    while(!d.atEnd()){
//        Schedule *s = new Schedule(this); 
//        d>>(*s);
//        schedule_list.append(s);
////        ui->listWidget_schedule->addItem(s->toString());
////        emit operateRequested("ui->listWidget_schedule","addItem",s->toString());
//    }
    unserSchedule(file_schedule.readAll());
//    emit businessEventOccurred(BusinessEvent::ScheduleUpdated,{{"schedule",scheduleStringList}});
//    emit scheduleUpdated(generateScheduleText());
    emit scheduleUpdated(serSchedule());
    file_schedule.close();

    //设置当前页
    QDir dir_empty_label1("config/empty/label1");
    if(dir_empty_label1.exists()){
//        ui->tabWidget->setCurrentIndex(0);
//        emit operateRequested("ui->tabWidget","setCurrentIndex",0);
        emit businessEventOccurred(BusinessEvent::PageIndexUpdated,{{"index",0}});
        is_first_launch = false;
    }
    else{
        dir_empty_label1.mkpath(".");
//        ui->tabWidget->setCurrentIndex(5);
//        emit operateRequested("ui->tabWidget","setCurrentIndex",5);
        emit businessEventOccurred(BusinessEvent::PageIndexUpdated,{{"index",5}});
        is_first_launch = true;
    }
    QDir dir_emty_label2("config/empty/label2");//不启用关机阻止
    if(!dir_emty_label2.exists()){
#ifdef Q_OS_WIN
//        ShutdownBlockReasonCreate((HWND)winId(),L"如果想要离线非挂起高速无限同步文件请不要关机 SyncTunnel 离线文件同步服务");
//        emit operateRequested("","ShutdownBlockReasonCreate","");
#endif
    }
    QDir dir_empty_label3("config/empty/label3");//DFHN客户端
    if(dir_empty_label3.exists()){
        device_flag = Communication::DFHNDevice;
        is_DFHN = true;
    }
    else{
        device_flag = Communication::WindowsDevice;
        winRun device_flag = Communication::WindowsDevice;
        androidRun device_flag = Communication::AndroidDevice;
        linuxRun device_flag = Communication::LinuxDevice;
        is_DFHN = false;
    }
    
    
    //信号槽绑定
//    connect(ui->listWidget_file,&QListWidget::doubleClicked,this,&MainWindow::on_folder_change); //已接管
//    connect(ui->listWidget_file,&QListWidget::customContextMenuRequested,this,&MainWindow::on_rightclick); //无需接管
//    connect(ui->actionOpen,&QAction::triggered,this,[this]{ //无需接管
//        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath()));
//    });
//    connect(ui->actionFolder,&QAction::triggered,this,[this]{/*QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators((current_dir).absolutePath())));*/QDesktopServices::openUrl(QUrl::fromLocalFile(current_dir.absolutePath()));});//无需接管
//    connect(ui->pushButton_settings_save,&QPushButton::clicked,this,&MainWindow::on_settings_saved);//已接管
//    connect(ui->actionupload_file,&QAction::triggered,this,[this]{sendFile();});//已接管
//    connect(ui->actionHangup,&QAction::triggered,this,&MainWindow::on_hangup);//已接管
//    connect(ui->actionDownload,&QAction::triggered,this,&MainWindow::on_download);//已接管
//    connect(ui->actionSync_PAT,&QAction::triggered,this,[this]{//已接管
//        QJsonObject json;json.insert("pat",ui->lineEdit_settings_githubPAT->text());
//        send(QJsonDocument(json).toJson());
//    });
//    connect(ui->checkBox_settings_ipv6,&QCheckBox::clicked,this,[this](bool isCheck){//不要了
//        if(isCheck)QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments()<<("-ipv6"));
//        else{
//            QStringList l=QApplication::arguments();l.removeAll("-ipv6");
//            QProcess::startDetached(QApplication::applicationFilePath(),l);
//        }close();
//    });
//    connect(ui->actionRefresh,&QAction::triggered,this,[this]{show_dir();});//无需接管
//    connect(ui->pushButton_switchProxy,&QPushButton::clicked,this,&MainWindow::on_proxy);//不要了
//    connect(process_proxy,&QProcess::readyRead,this,[this]{ui->textBrowser_proxy->append(QString::fromLocal8Bit(process_proxy->readAll()));});//不要了
//    connect(ui->tableWidget_deviceList,&QTableWidget::doubleClicked,this,)
//    connect(&timer_savePower,&QTimer::timeout,this,[this]{//无需接管
//        label_status->setText(QString(tr("SyncTunnel正在等待文件传输请求……\n如果没有请求，计算机将会在%1秒后关闭\n您也可以用Alt+F4关闭此窗口后手动关机")).arg(/*QTime().addMSecs(timer_savePower_finish.remainingTime()).toString("HH时mm分ss秒"))*/timer_savePower_finish.remainingTime()/1000));
//    });
//#ifdef Q_OS_WIN
//    connect(&timer_savePower_finish,&QTimer::timeout,this,[this]{//无需接管
//        ShutdownBlockReasonDestroy((HWND)winId());
//        QProcess::startDetached("shutdown",{"-s","-t","10"});(void)this;
//    });
//    connect(ui->actionShutdown,&QAction::triggered,this,[this]{ShutdownBlockReasonDestroy((HWND)winId());is_accept_shutdown=true;QProcess::startDetached("shutdown",{"-s","-t","10"});});//无需接管
//#endif
//    connect(ui->actionShutdown_current,&QAction::triggered,this,[this]{//已接管
//        int index = ui->tableWidget_deviceList->currentRow();
//        if(index==-1){
//            QMessageBox::information(this,tr("关闭选中的设备"),tr("请先选中一个设备！"));return;
//        }
//        m_communication->send(clients[index],encode("{\n    \"cmd\":\"shutdown -s -t 10\"\n}"));
//    }); 
//    connect(ui->tableWidget_deviceList,&QTableWidget::customContextMenuRequested,this,&MainWindow::on_rightclick_deviceList);  //无需接管
//    connect(ui->actionTest_RTT,&QAction::triggered,this,&MainWindow::on_test_rtt);//已接管
//    connect(ui->pushButton_settings_requestUI,&QPushButton::clicked,this,[]{//无需接管
//        QUrl url("mailto:nnpyro2@outlook.com");
//        QUrlQuery query;
//        query.addQueryItem("subject","SyncTunnel申请限定款UI");
//        query.addQueryItem("body",QString("您好！\n我是SyncTunnel的用户，我想要申请(填写您需要申请的界面样式 金色流光/银色星辰)UI样式。\n\n我对软件开发创作做出了如下突出贡献\n（请填写此处。或者您可以填写您是前50位下载本软件的用户）\n\n我的设备唯一ID：\n%1").arg(QString(QSysInfo::machineUniqueId())));
//        url.setQuery(query);QDesktopServices::openUrl(url);
//    });
//    connect(trayIcon,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason reason){Q_UNUSED(reason);if(isMinimized())showNormal();if(isHidden())show();raise();});//无需接管
//    connect(ui->pushButton_copyId,&QPushButton::clicked,this,[this]{QApplication::clipboard()->setText(QSysInfo::machineUniqueId());QMessageBox::information(this,"SyncTunnel","复制成功！");});//无需接管
//    connect(ui->actionRequestFile,&QAction::triggered,this,[this]{if(ui->tableWidget_deviceList->currentRow()==-1){QMessageBox::warning(this,"错误","请先选中一个设备");return;} send("REQ_FILE",1,ui->tableWidget_deviceList->currentRow());label_status->setText("正在等待发送方响应...");});//已接管
    // connect(&timer_keepAlive,&QTimer::timeout,this,[this]{if(chunks.empty()&&currentFileMap.empty())send("KEEP_ALIVE");});
//    connect(ui->actionRemoteCopyFile, &QAction::triggered, this, [this] {int currentRow = ui->tableWidget_deviceList->currentRow();if (currentRow == -1) {QMessageBox::warning(this, "错误","你需要先选中一个设备");return;}copy_remote_file(currentRow);});//无需接管
//    connect(ui->actionSend_message,&QAction::triggered,this,[this]{int index=ui->tableWidget_deviceList->currentRow();if(index==-1){QMessageBox::warning(this,"发送测试消息","请先选中一个设备");return;} QString msg=QInputDialog::getText(this,"发送测试消息","请输入测试消息：");QJsonObject json;json.insert("test_msg",QString(msg.toUtf8()));send(QJsonDocument(json).toJson(),1,index);});
//    connect(ui->actionStart_remote,&QAction::triggered,this,[this]{int index=ui->tableWidget_deviceList->currentRow();if(index==-1){QMessageBox::warning(this,"远程控制","请先选中一个设备");return;} send("{\n    \"opt\":\"start_remote\"\n}",1,index);ui->tabWidget->setCurrentIndex(2);remote_device=clients[index];});
//    connect(&timer_remote_sending,&QTimer::timeout,this,[this]{
//        QPixmap sc=QGuiApplication::primaryScreen()->grabWindow(0);QImage img=sc.toImage();int x=QCursor::pos().x(),y=QCursor::pos().y();for(int i=x-5;i<=x-5;i++)for(int j=y;j<=y-5;j++)img.setPixel(i,j,qRgb(255,0,0));
//        QImage i1,i2,i3,i4;//左上右上左下右下
//        int ax=sc.width(),ay=sc.height(),cx=ax/2,cy=ay/2;
//        i1=img.copy(0,0,cx,cy).convertToFormat(QImage::Format_RGB16);i2=img.copy(cx,0,cx,cy).convertToFormat(QImage::Format_RGB16);
//        i3=img.copy(0,cy,cx,cy).convertToFormat(QImage::Format_RGB16);i4=img.copy(cx,cy,cx,cy).convertToFormat(QImage::Format_RGB16);
//        QByteArray a1,a2,a3,a4;QBuffer b1(&a1),b2(&a2),b3(&a3),b4(&a4);
//        i1.save(&b1,"JPEG",15);i2.save(&b2,"JPEG",15);i3.save(&b3,"JPEG",15);i4.save(&b4,"JPEG",15);
        
//        int n=clients.indexOf(remote_be_device);
//        if(QTime::currentTime().second()%2==0){
//                            send("REMOTE_IMG_1_"+a1,1,n);
//        QThread::msleep(50);send("REMOTE_IMG_2_"+a2,1,n); 
//        QThread::msleep(50);send("REMOTE_IMG_3_"+a3,1,n); 
//        QThread::msleep(50);send("REMOTE_IMG_4_"+a4,1,n);}/*QThread::msleep(40);*/ 
//        else{
//                            send("REMOTE_IMG_4_"+a4,1,n);
//        QThread::msleep(50);send("REMOTE_IMG_3_"+a3,1,n); 
//        QThread::msleep(50);send("REMOTE_IMG_2_"+a2,1,n); 
//        QThread::msleep(50);send("REMOTE_IMG_1_"+a1,1,n);}
            
//        ndb<<"发送屏幕截图";
        
//    });
//    connect(ui->pushButton_remote_stop,&QPushButton::clicked,this,[this]{send("STOP_REMOTE",1,clients.indexOf(remote_device));});
//    connect(ui->pushButton_remote_left,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mleft\"\n}");});
//    connect(ui->pushButton_remote_right,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mright\"\n}");});
//    connect(ui->pushButton_remote_up,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mup\"\n}");});
//    connect(ui->pushButton_remote_down,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mdown\"\n}");});
//    connect(ui->pushButton_remote_mid,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mmid\"\n}");});
//    connect(ui->actionOpen_DriveCrypto,&QAction::triggered,this,[]{QProcess::startDetached("DriveCrypto.exe");});//无需接管
//    connect(ui->pushButton_settings_getDefaultPAT,&QPushButton::clicked,this,[this]{
//       QNetworkAccessManager *manager = new QNetworkAccessManager(this);
//       QNetworkRequest request;
//       request.setUrl(QUrl("https://nnpyro.netlify.app/synctunnel-interface/github_pat.txt"));
//       request.setHeader(QNetworkRequest::UserAgentHeader,"nnpyro SyncTunnel vbeta-x.x");
//       QNetworkReply *reply=manager->get(request);
//       QEventLoop loop;
//       connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
//       loop.exec();
//       if(reply->error() != QNetworkReply::NoError) QMessageBox::critical(this,"错误","错误"+reply->errorString());
//       else {ui->lineEdit_settings_githubPAT->setText(reply->readAll());ui->lineEdit_settings_gitubUser->setText("nnpyro1");}
//    });
//    connect(shortcut_debug,&QShortcut::activated,this,[this]{is_debug=is_debug?false:true;restartDebug();QMessageBox::information(this,"debug",QString("您已%1调试模式").arg(is_debug?"进入":"离开"));});无需接管
//    connect(ui->checkBox_info_viewLowDelay,&QCheckBox::clicked,this,[this]{if(ui->checkBox_info_viewLowDelay->isChecked())axis_y->setRange(0,100);else axis_y->setRange(0,SEND_MAX_DELAY);});//废弃
//    connect(ui->pushButton_settings_recordLog,&QPushButton::pressed,this,[this]{if(!QDir("logs").exists())QDir("logs").mkpath(".");logFile->open(QFile::WriteOnly);ninfo<<"类"<<this<<"将日志输出重定向到文件";output_to_file = true;qInstallMessageHandler(static_cast<QtMessageHandler>(log));ninfo<<"日志输出重定向成功！";});//无需接管
//    connect(ui->actionAdd_file,&QAction::triggered,this,[this]{QString dir=QFileDialog::getOpenFileName(this);QFile(dir).copy(current_dir.absolutePath());});//可选同步方面无需接管，日程方面需set函数
//    connect(ui->actionIncremental_sync_enable,&QAction::triggered,this,[this]{
//        incremental_sync_set.insert(QDir("files").relativeFilePath(current_dir.filePath(ui->listWidget_file->currentItem()->text())));ninfo<<"var:incremental_sync_set:"<<incremental_sync_set;
//        ui->listWidget_incremental->clear();foreach(QString i,incremental_sync_set){ui->listWidget_incremental->addItem(i);show_dir();}});
//    connect(ui->actionIncremental_sync_disable,&QAction::triggered,this,[this]{
//        incremental_sync_set.remove(QDir("files").relativeFilePath(current_dir.filePath(ui->listWidget_file->currentItem()->text())));ninfo<<"var:incremental_sync_set:"<<incremental_sync_set;
//        ui->listWidget_incremental->clear();foreach(QString i,incremental_sync_set){ui->listWidget_incremental->addItem(i);}if(incremental_sync_set.empty())ui->listWidget_incremental->addItem("空 （所有文件都将被同步）");show_dir();});  
//    connect(ui->pushButton_file_syncAll,&QPushButton::clicked,this,[this]{incremental_sync_set.clear();ui->listWidget_incremental->clear();ui->listWidget_incremental->addItem("空 （所有文件都将被同步）");show_dir();});
//    connect(ui->pushButton_file_addSchedule,&QPushButton::clicked,this,[this]{dialog_schedule=new Dialog_schedule(this);
//        connect(dialog_schedule,&Dialog_schedule::saved,this,[this](Schedule *s){schedule_list.append(s);s->setInterval(5000);s->setEnabled(true);s->setAutoTrigger(true);
//                                                                                 connect(s,&Schedule::triggered,this,[this]{auto c=clients;c.removeAll(public_ip);sendFile(c);});});
//        dialog_schedule->exec();dialog_schedule->deleteLater();
//        ui->listWidget_schedule->clear();foreach(auto i,schedule_list)ui->listWidget_schedule->addItem(i->toString());
//        QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();
//    });
//    connect(ui->pushButton_file_deleteCurrentSchedule,&QPushButton::clicked,this,[this]{int i=ui->listWidget_schedule->currentRow();if(i==-1){QMessageBox::warning(this,tr("删除"),tr("请先选中一个日程"));return;}
//    schedule_list[i]->deleteLater();schedule_list.removeAt(i);ui->listWidget_schedule->clear();foreach(auto i,schedule_list)ui->listWidget_schedule->addItem(i->toString());
//    QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();});
//    connect(qApp,&QApplication::applicationStateChanged,this,[this](Qt::ApplicationState st){androidRun{
//            if(st == Qt::ApplicationSuspended){ninfo<<"应用程序退后台，自动关闭";m_signalling->exit();}
//            else if(st==Qt::ApplicationActive){m_signalling->deleteLater();m_signalling=new Signalling();
//                m_signalling->connectToHost(mqtt_server);
//                m_signalling->subscribe("nnpyro_syncTunnel/user_topics/" + user_name.toUtf8().toBase64());
//                m_signalling->setPwd(pwd.toUtf8());
//                m_signalling->setUser(public_ip,user_name);
//                clients = m_signalling->getUserList();}}});//已接管
//    connect(ui->actionExit_Application,&QAction::triggered,this,&MainWindow::close);
//    connect(ui->pushButton_settings_console,&QPushButton::clicked,this,[this]{if(!QDir("tools/").exists())QDir().mkpath("tools/");QFile::copy(":/rc/bin/Alacritty.exe","tools/Alacritty.exe");QProcess::startDetached("tools/Alacritty.exe",QStringList()<<"-e"<<QApplication::applicationFilePath()<<QApplication::arguments()<<"CON_MODE");close();});
//    connect(ui->commandLinkButton_route_page2,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(2);});
//    connect(ui->commandLinkButton_route_page3,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(3);});
//    connect(ui->commandLinkButton_route_page5,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(5);});
//    connect(ui->commandLinkButton_route_page6,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(6);});
//    connect(ui->pushButton_settings_mode_normal,&QPushButton::clicked,this,[this]{if(QMessageBox::warning(this,"高级设置-普通模式","这是高级设置，如果您不理解其中的含义，请不要设置。\n\n设置成普通模式后，设备将不会作为DFHN节点，无法在设备列表里便捷标记，因此无法从DFHN节点取下挂起的文件\n\n你确定要修改吗？",QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
//            QDir().rmdir("config/empty/label3");QProcess::startDetached(QApplication::applicationFilePath());close();
//        }});
//    connect(ui->pushButton_settings_mode_dfhn,&QPushButton::clicked,this,[this]{if(QMessageBox::warning(this,"高级设置-DFHN模式","这是高级设置，如果您不理解其中的含义，请不要设置。\n\n设置成DFHN模式后，设备将作为DFHN节点，在设备列表里特殊标记，用于将设备作为DFHN节点挂起文件。\n设置成DFHN模式后，可能会影响正常使用完整功能。\n\n你确定要修改吗？",QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
//            QDir().mkpath("config/empty/label3");QProcess::startDetached(QApplication::applicationFilePath());close();
//        }});
//    connect(ui->actionHang_up_file_to_dfhn,&QAction::triggered,this,[this]{QList<device> l;for(auto i : clients){if(i.flag==Communication::DFHNDevice)l.append(i);}if(l.empty())QMessageBox::warning(this,"挂起","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN");else sendFile(l);});
//    connect(ui->actionDownload_file_from_dfhn,&QAction::triggered,this,[this]{QList<device> l;for(auto i : clients){if(i.flag==Communication::DFHNDevice)l.append(i);}if(l.empty())QMessageBox::warning(this,"下载","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN");else send("REQ_FILE",1,clients.indexOf(l[0]));});
//    connect(ui->commandLinkButton_route_help,&QCommandLinkButton::clicked,this,[]{Dialog_help *h = new Dialog_help;h->exec();h->deleteLater();});
//    connect(&timer_refresh,&QTimer::timeout,this,[this]{show_dir();});
//    connect(ui->checkBox_file_autoSync,&QCheckBox::stateChanged,this,[this](int state){is_autoSync = (state==Qt::Checked);   settings.setValue("ApplicationSettings/isAutoSync",is_autoSync);});
    connect(&timer_autoSync,&QTimer::timeout,this,[this]{
        if(!is_autoSync)return;
        auto newFileHashMap = Utils::generateFileHashMap(syncFolder);
        //比较
        bool flag=false;
        for(auto it = newFileHashMap.constBegin();it!=newFileHashMap.end();it++){
            if (!fileHashMap.contains(it.key()) || it.value() != fileHashMap.value(it.key())) {
                flag = true;
                break;
            }
        }
        if(flag==true){
            ninfo<<"检测到文件修改，自动同步";
            fileHashMap=newFileHashMap;
            sendFile((lastSyncDst));
        }
        else{
            
        }
    });
//    connect(ui->actionRestart_all_applications,&QAction::triggered,this,[this]{m_transmissionengine->SPTP_sendCtrl("RESTART_NETWORK","",-2);});
    connect(m_storage,&Storage::progressUpdated,this,[this](Storage::ProcessingState st,double progress){
        ninfo<<"Step:"<<QMetaEnum::fromType<Storage::ProcessingState>().valueToKey(st)<<",Progress:"<<progress*100<<"%";
        emit businessEventOccurred(BusinessEvent::FileUploadProgressUpdated,{{"step",(int)st},{"progress",progress}});
    });
    
    //目录显示
    current_dir = QDir("files/");
    emit businessEventOccurred(BusinessEvent::CurrentDirUpdated,{{"value",current_dir.absolutePath()}});    
    
    
//    initNetwork(func_update);
    //初始化网络
    //网络部分
    //发起STUN
//    emit messageChanged(tr("正在获取公网IP……"));
//     emit businessEventOccurred(BusinessEvent::GettingPublicIp);
//     if(QDir("config/empty/label3").exists())QCoreApplication::processEvents();
//     public_ip = m_communication->stun();
//     public_ip.description=device_description;
//     public_ip.flag = device_flag;
//     if(public_ip == Communication::ipport{"",0} || QApplication::arguments().contains("-ipv6")){
//         QCoreApplication::processEvents();
        
//         //开始用IPv6
//         public_ip = m_communication->getIPv6();
        
//         if(public_ip == Communication::ipport{"",0}){
// //            emit messageBoxRequested(tr("获取公网IP"),tr("您的设备获取IPv4公网IP失败，且不支持IPV6。\n请如果是第一次发生，请重试。\n如果多次发生，请只使用文件挂起功能或更换网络环境。"),MessageBoxType::Warning);
// //            QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());
// //            QCoreApplication::quit();
// //            abort();
// //            QApplication::processEvents();
//             emit businessEventOccurred(BusinessEvent::PublicIpGetFailed);
//             Utils::restart();
//         }
//     }
// //    emit messageChanged(tr("正在获取上线用户列表……"));
//     emit businessEventOccurred(BusinessEvent::GettingDeviceList);
//     ninfo<<"var:public ip="<<public_ip;
//     //MQTT
//     // m_signalling->connectToHost(/*{"broker.emqx.io",1883}*/mqtt_server);
//     // m_signalling->subscribe("nnpyro_syncTunnel/user_topics/" + user_name.toUtf8().toBase64());
//     // m_signalling->setPwd(pwd.toUtf8());
//     // m_signalling->setUser(public_ip,user_name);
//     // clients = m_signalling->getUserList();//获取用户列表
//     m_signalling->setPublicIp(public_ip);
//     m_signalling->setMqttBroker(mqtt_server.ip,mqtt_server.port);
//     m_signalling->setPassport(user_name,pwd);
//     if(!m_signalling->start()) ncritical<<"Unable to start m_signalling";
//     m_signalling->registerOnline();
//     clients = m_signalling->getAllDevices();
//     if(1)foreach(auto i,clients)qDebug()<<i;
// //    emit messageChanged(tr("加载成功"));
//     emit deviceListUpdated(clients);
    
//     //等待直到用户列表获取完成
//     // QEventLoop el1;
//     // connect(m_signalling,&Signalling::on_userlist_updata,&el1,&QEventLoop::quit);
// //    el1.exec(QEventLoop::ExcludeUserInputEvents);
// //    QCoreApplication::processEvents();
    
//     //调试
// //    if(1)ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));
// //    if(1)foreach(auto i,clients)ui->textBrowser_debug1->append(i);
    
//     //设置文件挂起
//     // m_storage->setUser(user_github_name,user_github_PAT);
//     // m_storage->setUserId(user_name);
//     m_storage->setPassport(user_name,pwd);
    
//     //接管communicaion
//     ndb<<"接管communication";
//     m_transmissionengine = new TransmissionEngine(m_communication,user_name,pwd,public_ip,this);
//     m_transmissionengine->setClients(clients);
//     connect(m_transmissionengine,&TransmissionEngine::communicationReadyRead,this,&BusinessLogic::on_readyRead);
//     connect(m_transmissionengine,&TransmissionEngine::SPTP_readyRead,this,&BusinessLogic::on_SPTP_readyRead);
//     connect(m_transmissionengine,&TransmissionEngine::messageChanged,this,[this](QString msg){emit messageChanged(msg);});
//     connect(m_transmissionengine,&TransmissionEngine::SPTP_sendFinished,this,[this]{emit businessEventOccurred(BusinessEvent::SendedSuccessfully);playSound(QUrl("qrc:/rc/audio/file_send_successfully.wav"));});
//     connect(m_transmissionengine,&TransmissionEngine::SPTP_ctrlMsgReceived,this,&BusinessLogic::on_SPTP_ctrlMsg_received);
//     connect(m_signalling,&Signalling::deviceUpdated,this,[this]{
//         clients = m_signalling->getAllDevices();if(1)foreach(auto i,clients)qDebug()<<i;
//         QTimer::singleShot(3000,[this]{
//             for(int i=0;i<5;i++){
//                 send("{\n    \"hole\":1\n}");
//                 QThread::msleep(20);
//                 send("{\n    \"hole\":2\n}");
//                 QThread::msleep(20);
//             }
//         });
// //        emit messageChanged(tr("用户列表更新成功"));
// //        ui->tableWidget_deviceList->clearContents();ui->tableWidget_deviceList->setRowCount(0);ndb<<"IN";
//         if(m_transmissionengine)m_transmissionengine->setClients(clients);
        
// //        foreach(auto i,clients){        //表格设置
// //            int r=ui->tableWidget_deviceList->rowCount();
// //            ui->tableWidget_deviceList->insertRow(r);
// //            ui->tableWidget_deviceList->setItem(r,0,new QTableWidgetItem((i.flag==Communication::DFHNDevice?"**":"") + user_name));
// //            ui->tableWidget_deviceList->setItem(r,1,new QTableWidgetItem(i.ip));
// //            ui->tableWidget_deviceList->setItem(r,2,new QTableWidgetItem(QString::number(i.port)));
// //            ui->tableWidget_deviceList->setItem(r,3,new QTableWidgetItem((i.flag==Communication::DFHNDevice?"**":"")+i.description+"("+QMetaEnum::fromType<Communication::DeviceFlag>().valueToKey(i.flag)+")"));
// //        }
// //        if(1){ui->textBrowser_debug1->clear();ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));foreach(auto i,clients)ui->textBrowser_debug1->append(i.toFullString());}
//         emit deviceListUpdated(clients);
//     });
//     // connect(m_signalling,&Signalling::errorOccurred,this,[=](QString err){emit businessEventOccurred(BusinessEvent::SignallingFailed,{{"error",err}});});
//     connect(m_transmissionengine,&TransmissionEngine::sendInfoChanged,this,&BusinessLogic::sendInfoChanged);
//     //接管remotecontrolengine
    m_remotecontrolengine=new RemoteControlEngine(m_rpepengine,this);
//     //临时测试用：
//     // connect(m_remotecontrolengine,&RemoteControlEngine::remoteScreenChanged,this,[this](QImage pm){emit businessEventOccurred(BusinessEvent::Debug,{{"data",pm}});});
    
//     //打洞
//     for(int i=0;i<5;i++){
//         send("{\n    \"hole\":1\n}");
//         send("{\n    \"hole\":2\n}");
//         QThread::msleep(50);
//     }
// //    emit messageChanged(tr("加载成功"));
//     emit businessEventOccurred(BusinessEvent::LoadedSuccessfully);
    
    //初始化网络
    m_rpepengine->setMqttBroker(mqtt_server);
    m_rpepengine->setUsername(user_name);
    m_rpepengine->setPassword(pwd);
    connect(m_rpepengine,&RpepEngine::eventOccurred,this,[this](RpepEngine::Event e,QVariantMap args){
        switch(e){
        case RpepEngine::Event::GettingPublicIp:
            emit businessEventOccurred(BusinessEvent::GettingPublicIp);
            break;
        case RpepEngine::Event::Error:
            emit businessEventOccurred(BusinessLogic::BusinessEvent::ErrorOccurred,args);
            break;
        case RpepEngine::Event::RegisteringOnline:
            emit businessEventOccurred(BusinessEvent::GettingDeviceList);
            break;
        case RpepEngine::Event::Ready:
            emit businessEventOccurred(BusinessEvent::LoadedSuccessfully);
            break;
        case RpepEngine::Event::Punch:
            emit businessEventOccurred(BusinessEvent::ConnectedSuccessfully,{{"ipport",args["id"]}});
            break;
        }
    });
    connect(m_rpepengine,&RpepEngine::deviceUpdated,this,[this]{ninfo<<"设备列表更新";public_ip=m_rpepengine->getPublicIp();clients=m_rpepengine->getAllDevices();emit deviceListUpdated(clients);});
    connect(m_rpepengine,&RpepEngine::controlReceived,this,&BusinessLogic::onControlReceived);
    connect(m_rpepengine,&RpepEngine::dataReceived,this,&BusinessLogic::onDataReceived);
    {
        auto ret = m_rpepengine->init();
        if(!ret){
            emit businessEventOccurred(BusinessEvent::ErrorOccurred,{{"error",ret.errorMessage}});
        }
    }
    connect(m_rpepengine,&RpepEngine::congestionControlInfoUpdated,this,&BusinessLogic::sendInfoChanged);
    
    
    //参数处理
    ninfo<<"Application arguments:"<<QApplication::arguments();
    auto args = QApplication::arguments();
    
    if(args.contains("-proxy")){//加速
//        emit messageBoxRequested("加速功能已禁用","O(∩_∩)O哈哈~，我知道你的小心思\n但是，网络加速功能 已 经 被 禁 用 了 ！",MessageBoxType::Information);
//        on_proxy();
        emit businessEventOccurred(BusinessEvent::ProxyBanned);
    }
    if(args.contains("-ipv6")){//强制使用ipv6
//        ui->checkBox_settings_ipv6->setChecked(true);
//        emit operateRequested("ui->checkBox_settings_ipv6","setChecked",true);
//        emit businessEventOccurred(BusinessEvent::Ipv6UsageStateUpdated,{{"state",true}});
        use_ipv6=true;
    }
    if(args.contains("CON_MODE")){
        output_to_file=false;
        qInstallMessageHandler(static_cast<QtMessageHandler>(log));
    }
    
    //其他部分
//    trayIcon->show();
    timer_keepAlive.start(15000);
    timer_autoSync.start(10000);//每隔10秒刷新
    RUN_IN_MAIN_THREAD(winRun if(!json_settings["disable_notice"].toBool())QProcess::startDetached("Noticer.exe"););
    
//    QTimer::singleShot(1000,this,[this]{raise();});
    /*if(is_first_launch){
        QMetaObject::invokeMethod(this,[this]{
            int button1 = QMessageBox::information(this,"向导","为了节省您宝贵的时间，我们可以为您展示一个快速配置向导，引导您快速配置软件。请问您需要吗？",QMessageBox::Yes|QMessageBox::No);
            if(button1==QMessageBox::Yes){
                QString qss = qApp->styleSheet();
                qApp->setStyleSheet("");
                Wizard_startup *w = new Wizard_startup(this);
                connect(w,&Wizard_startup::settingsSaved,this,[this](QString username,QString pwd,QString gh,QString ghpat){
                    ui->lineEdit_settings_username->setText(username);
                    ui->lineEdit_settings_pwd->setText(pwd);
                    ui->lineEdit_settings_githubPAT->setText(ghpat);
                    ui->lineEdit_settings_gitubUser->setText(gh);
                    ui->pushButton_settings_save->click();
                });
                w->exec();
                qApp->setStyleSheet(qss);
            }
        },Qt::QueuedConnection);
    }*/
    if(!QDir("files/").exists())QDir("files").mkpath(".");
    
    timer_refresh.start(2500);
    fileHashMap=Utils::generateFileHashMap(syncFolder);
    ndb<<syncFolder.absolutePath();
    
    //同步设置
    emit businessEventOccurred(BusinessEvent::SettingsUpdated,{{"username",user_name},{"password",pwd},{"description",device_description},{"ipv6usage",use_ipv6},{"disablenotice",json_settings["disable_notice"].toBool()}});
    
    //发送统计信息
    stat();
}


void BusinessLogic::destory(){
    ndb<<"运行了销毁函数";
    //定时器
    timer_keepAlive.stop();
    timer_autoSync.stop();
    timer_refresh.stop();
    timer_is_uploading.stop();
    timer_clear_currentFileMap.stop();
    //关闭一些东西
    if (logFile->isOpen()) {
        logFile->close();
    }
    // if(m_signalling->isAvailable()){
    //     m_signalling->registerOffline();
    //     m_signalling->stop();
    // }
    m_rpepengine->destroy();
    //销毁对象
    // delete m_communication;             m_communication = nullptr;
    // delete m_signalling;                m_signalling = nullptr;
    delete m_storage;                   m_storage = nullptr;
    // delete m_transmissionengine;        m_transmissionengine = nullptr;
    
    m_rpepengine->deleteLater();m_rpepengine=nullptr;
}

// void BusinessLogic::send(QByteArray msg, bool e, int d){
//     m_transmissionengine->send(msg,e,d); 
// }


Result BusinessLogic::sendFile(QSet<devid_t> dst,QSet<QString> incremental_sync_set){
    if(dst.empty()){
        return Result("请指定传输目标");
    }
    lastSyncDst=dst;
    // m_transmissionengine->SPTP_send(Utils::mergeFile(QDir("files/"),incremental_sync_set),dst);
    Result res = m_rpepengine->transfer(Utils::mergeFile(syncFolder,incremental_sync_set),dst);
    if(!res){
        ncritical<<res.errorMessage;
    }
    return res;
}


bool BusinessLogic::checkSkin(BusinessLogic::skinType skin){
    //调整：
    return true;
    //不进行后面
    
    if(skin<Silver) return true;
    
    //查找请求地址
    QUrl req_url;
    switch (skin) {
    case Golden:
        req_url = QUrl("https://cfpages.nnpyro.fwh.is/synctunnel-interface/skin-golden-1.0.txt");
        break;
    case     Silver:
        req_url = QUrl("https://cfpages.nnpyro.fwh.is/synctunnel-interface/skin-silver-1.0.txt");
        break;
    default:
        ninfo<<"ERROR Unexcept skin"<<skin;
    }   
    
    
    //请求
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(req_url);
    QNetworkReply *reply = manager->get(request);
    
    //等待
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    QTimer::singleShot(6000,&loop,&QEventLoop::quit);
    loop.exec();
    
    //处理
    if(reply->error() != QNetworkReply::NoError){
//        QMessageBox::critical(this,tr("样式查询"),tr("连接至服务器错误，无法使用限定款UI界面。\nError:")+reply->errorString());
//        emit messageBoxRequested(tr("样式查询"),tr("连接至服务器错误，无法使用限定款UI界面。\nError:")+reply->errorString(),MessageBoxType::Critical);
        emit businessEventOccurred(BusinessEvent::SkinCheckFailed,{{"error",reply->errorString()}});
        delete manager;
        return false;
    }   
    QString msg = reply->readAll();
    delete manager;
    QTextStream stm(&msg);
    QString machineID = QCryptographicHash::hash(QSysInfo::machineUniqueId(),QCryptographicHash::Sha256);
    reply->deleteLater();
    for(;!stm.atEnd();){
        QString id = stm.readLine();
        if(id==machineID){
            delete manager;
            return true;
        }
    }
    return false;
    
}


//---------- 以下是公有槽函数 ----------


void BusinessLogic::on_folder_change(QDir current_dir){
//    current_dir = QDir(current_dir.filePath(current_dir));
    this->current_dir=current_dir;
    emit businessEventOccurred(BusinessEvent::CurrentDirUpdated,{{"value",current_dir.absolutePath()}});
}


void BusinessLogic::on_settings_saved(QString username_,QString pwd_,QString mqttServer_,int mqttPort_,QString githubUser_,QString githubPat_,QVariant skin_,bool recordLog_,bool disableNotice_,QString description_){
    bool restart_flag = false;
    
    if(user_name!=username_||pwd!=pwd_){//保存用户名密码
        auto un=username_,p=pwd_;
        if(un.size()<8||p.size()<8){
//            emit messageBoxRequested("无法设置用户名密码","用户名密码过短。要求用户名和密码不少于8字符",MessageBoxType::Warning);
            emit businessEventOccurred(BusinessEvent::UsernamePasswordTooShort);
            return;
        }
//         if(user_name!=username_ && !QDir("config/empty/label1/ext-label-private/label-SyncTunnel-Username-uploaded/").exists()){
// //            int btn = QMessageBox::information(this,"首次设置用户名防抢注验证","为同一用户防止恶意抢注、占用多个用户名、保障所有用户公平使用P2P功能，我们需要完成一次匿名验证：\n"" 仅上传您用户名的匿名加密串（攻击者不可能还原用户名明文，无任何个人信息）；\n"" 仅首次设置用户名时操作一次，后续不再上传任何数据；\n"" 数据存储于国内服务器，在验证用户名不是恶意抢注或占有后会尽快自动删除，数据最长留存一年（可联系nnpyro2@outlook.com删除）\n重要：若拒绝验证，您只能使用默认用户名+自定义密码使用软件，默认用户名为多用户共享，安全性低，若密码发生碰撞，您的个人数据可能会泄露，强烈不推荐使用。若使用，请设置强密码，并对自己的数据安全负全责\n\n""是否确认完成验证并保存用户名？",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
// //            if(btn==QMessageBox::No){
// //                QMessageBox::information(this,"用户名未验证","用户名未经过防抢注验证，未保存，自动使用默认用户名");
// //                ui->lineEdit_settings_username->setText("DefaultUser");
// //                if(p.size()<15){
// //                    QMessageBox::warning(this,"强烈建议","您正在使用默认用户名，默认用户名安全性极低，我们强烈建议您的密码长度大于15字符！！！！！\n\n\n或者您可以直接设置一个专属用户名！使用默认用户名的安全性极低，极易发生密码碰撞/数据泄露的重大安全事故，如果您使用默认用户名，请对自己的数据安全负全责！");
// ////                    return;
// //                }
// //            }
//             if(0){}
//             else{
// //                emit messageChanged("正在进行验证……");
//                 emit businessEventOccurred(BusinessEvent::UploadingFirstLaunchInformation);
//                 auto manager = new QNetworkAccessManager(this);
//                 QNetworkRequest request;
//                 //构造请求
//                 //            request.setRawHeader("Authorization","Bearer github_pa""t_11BF");
//                 request.setHeader(QNetworkRequest::UserAgentHeader,"NNPYRO SyncTunnel Service");
//                 //            request.setRawHeader("Accept", "application/vnd.github.v3+json");
//                 request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//                 QJsonObject json;
                
//                 json["message"]="SyncTunnel Upload.";
// //                json["access_token"]=SYNCTUNNEL_INTERFACE_W_ACCESS_TOKEN;
//                 request.setRawHeader("PRIVATE-TOKEN",SYNCTUNNEL_INTERFACE_W_ACCESS_TOKEN);
//                 json["branch"]="username";
//                 //设置统计文件
//                 request.setUrl(QUrl(QString("https://api.gitcode.com/api/v5/repos/2501_93498940/synctunnel-interface-w/contents/users/%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))));
//                 json["content"]=QString(QString("Time:%1\nUser-Name:None\n").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")));
//                 QNetworkReply *reply = manager->post(request,QJsonDocument(json).toJson());
//                 //等待响应
//                 QEventLoop loop;
//                 connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
//                 QTimer::singleShot(15000,&loop,&QEventLoop::quit);
//                 loop.exec();
//                 if(reply->error() != QNetworkReply::NoError || !reply->isFinished()){
// //                    emit messageBoxRequested("错误","错误：无法请求数据到服务器。详细信息："+reply->errorString()+"\n但是这不影响您继续正常使用软件。单击“确定”继续保存",MessageBoxType::Critical);
//                     emit businessEventOccurred(BusinessEvent::UploadingFirstLaunchInformationFailed,{{"error",reply->errorString()}});
//                     //允许用户暂时使用
// //                    return;
//                 }
//                 else{
// //                    QMessageBox::information(this,"成功","验证成功！您的用户名设置成功，应用程序自动重启。");
//                     QDir("config/empty/label1/ext-label-private/label-SyncTunnel-Username-uploaded/").mkpath(".");//下次不上传
//                 }
//                 reply->deleteLater();
//                 manager->deleteLater();
//             }
//         }
        user_name = username_;
        pwd = pwd_;
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/1.nprivate0");
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QByteArray iv;iv.resize(16);
        QRandomGenerator::global()->generate(iv.data(),iv.data()+16);
        auto buf = QString("%1\n%2").arg((QString)user_name.toUtf8().toBase64()).arg(/*(QString)pwd.toUtf8().toBase64()*/(QString)((iv+QAESEncryption::Crypt(QAESEncryption::AES_256,QAESEncryption::CBC,pwd.toUtf8(),SYNCTUNNEL_CRPT_KEY,iv)).toBase64())).toUtf8();
        f.write(buf);//写入
        f.close();
        restart_flag=true;
    }
    if(mqttServer_ != mqtt_server.ip || mqttPort_ != mqtt_server.port){
        mqtt_server = ipport{mqttServer_,(quint16)mqttPort_};
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/2.nprivate0");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QString("%1\n%2").arg(QString(mqtt_server.ip.toUtf8().toBase64())).arg(mqtt_server.port).toUtf8());
        f.close();
        restart_flag=true;
    }
    if(githubUser_ != user_github_name || githubPat_ != user_github_PAT){
        user_github_name = githubUser_;
        user_github_PAT = githubPat_;
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/3.nprivate0");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QString("%1\n%2").arg(QString(user_github_name.toUtf8().toBase64())).arg(QString(user_github_PAT.toUtf8().toBase64())).toUtf8());
        f.close();
        restart_flag=true;
    }
    if(skin_ != currentSkin){
        currentSkin=(skinType)skin_.toInt();
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/4.nprivate0");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QString::number(currentSkin).toUtf8());
        f.close();
        restart_flag=true;
    }
    if(recordLog_!=json_settings["use_log"].toBool() || disableNotice_!=json_settings["disable_notice"].toBool() || description_!=device_description){
        json_settings["use_log"]=recordLog_;
        json_settings["disable_notice"]=disableNotice_;
        json_settings["description"]=description_;
        QFile f("config/config.json");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QJsonDocument(json_settings).toJson());
        f.close();
        restart_flag=true;
    }
//    if(ui->comboBox_settings_language->currentIndex() != current_language){
//        switch (ui->comboBox_settings_language->currentIndex()) {
//        case language_chinese:
//            QApplication::removeTranslator(translator);
//            if(translator)delete translator;
//            translator = nullptr;
//            ui->retranslateUi(this);
//            break;
//        case language_english:
//            translator = new QTranslator(this);
//            translator->load(":/rc/translations/trans_en_US.qm");
//            QApplication::installTranslator(translator);
//            ui->retranslateUi(this);
//            break;
//        }
//        current_language = (language)ui->comboBox_settings_language->currentIndex();
//        ninfo<<"language:"<<current_language;
//    }
    
    
    if(restart_flag){//重启
        // m_signalling->exit();//发布关闭消息
        // QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());//重启
        // QCoreApplication::quit();
        Utils::restart();//软重启
    }
}

void BusinessLogic::on_hangup(){ //在找到可用替代方案之前，禁止文件挂起
//    if((GetAsyncKeyState('F')&0x8000)==0){
//        emit messageBoxRequested("文件挂起","现在文件挂起功能非常的坏，强烈不建议使用，大概率失败、卡死、闪退、崩溃，建议用DFHN（详见设置）作为替代品。如果你硬要用文件挂起，那么你就按住F再来点击按钮。记住，目前文件挂起强烈不建议使用。",MessageBoxType::Information);
//        return;
//    }
    
//    emit messageChanged(tr("正在挂起文件"));
//    QByteArray fileContent = encode(Utils::mergeFile(QDir("files/")));
//    if(m_storage->upload(fileContent)){
//        emit messageChanged(tr("文件挂起成功"));
//        emit messageBoxRequested("文件挂起",tr("文件挂起成功！"),MessageBoxType::Information);
//    }
//    else{
//        emit messageChanged(tr("文件挂起失败"));
//        messageBoxRequested("文件挂起",tr("文件挂起失败！\n\n可能是由于网络波动、服务器关闭等原因。\n文件不能重复挂起，如果第一次文件挂起成功那么第二次重复挂起必定失败。建议检查文件挂起状态是否为“已挂起”。\nGitHub限制每小时最多5000请求，请勿频繁操作文件。（解决办法参见帮助文档）\n\n详细信息参见帮助文档。"),MessageBoxType::Critical);
//    }

    QByteArray fileContent = encode(Utils::mergeFile(QDir(syncFolder)));
    auto result = m_storage->upload(/*fileContent*/"12345上山打老虎");
    QVariantMap map;
    if(!result.is_succeeded){
        map["error"]=result.errorMessage;
    }
    emit businessEventOccurred(BusinessEvent::FileUploadingFinished,map);
}


void BusinessLogic::on_download(){
//    emit messageBoxRequested("文件挂起","现在文件挂起功能非常的坏，强烈不建议使用，大概率失败、卡死、闪退、崩溃,所以还是别用了，考虑一下DFHN（详见设置）作为替代品",MessageBoxType::Information);
    emit businessEventOccurred(BusinessEvent::FileUploadingFinished,{{"error","开发者偷了点懒，目前这个功能还没有编完~\n解决方案1:上传小于50MB的文件\n解决方案2::可以到Github上给本项目贡献一个PR来完善这个功能"}});
}


void BusinessLogic::on_sync_pat(){
    // QJsonObject json;json.insert("pat",user_github_PAT);
    // send(QJsonDocument(json).toJson());
}


void BusinessLogic::on_shutdown_current(int id){
    int index = id;
    if(index==-1){
//        emit messageBoxRequested(tr("关闭选中的设备"),tr("请先选中一个设备！"),MessageBoxType::Warning);return;
        return;
    }
    // m_communication->send(clients.value(index),encode("{\n    \"cmd\":\"shutdown -s -t 10\"\n}"));
    m_rpepengine->sendControl("SHUTDOWN","",id);
}


void BusinessLogic::on_test_rtt(){
//     rtt_result.clear();
//     elapsed_rtt.start();
    
//     //发送测试信息
//     // send("{\n    \"opt\":\"rtt_test\"\n}");
//     // m_rpepengine->sendControl("TEST_RTT","",)
    
// //    emit messageChanged("正在测试RTT");
//     emit businessEventOccurred(BusinessEvent::TestingRTT);
//     QEventLoop loop;
//     QTimer timer;
//     connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
//     timer.start(2000);
//     loop.exec();
// //    emit messageChanged("RTT测试成功");
//     emit businessEventOccurred(BusinessEvent::RTTTestSuccessfully);
    
// //    QString result_str;
//     QList<QVariantMap> rttResult;
//     for(auto it=rtt_result.begin();it!=rtt_result.end();it++){
//         auto c=clients.value(it.key());
// //        result_str.append(QString("device%4:%1\tdelay:%2\tRTT:%3\n").arg(clients[it.key()]).arg(it.value()).arg(it.value()/2).arg(it.key()));
//         rttResult.append({
//                              {"id",it.key()},
//                              {"ip",c.ip},
//                              {"port",c.port},
//                              {"rtt",it.value()/2.},
//                              {"delay",it.value()}
//                          });
//     }
// //    emit messageBoxRequested("RTT测试结果","下面是本轮RTT测试的结果：\n\n"+result_str,MessageBoxType::Information);
//     emit rttTestResultUpdated(rttResult);
}


Result BusinessLogic::on_request_file(int index){
    if(!clients.contains(index)){
        return Result(QString("Index %1 out of range").arg(index));
    }
    // send("REQ_FILE",1,index);
    m_rpepengine->sendControl("REQ_FILE","",index);
//    emit messageChanged("正在等待发送方响应...");
    emit businessEventOccurred(BusinessEvent::WaitingForResponse);
    return Result();
}


void BusinessLogic::on_copy_remote_file_operation_requested(QString subdir, int index){// ### 需要重写 ###
    if(!subdir.startsWith("|")){
        // send(QString("{\n    \"get_folderList\":\"%1\"\n}").arg(subdir).toUtf8(),1,index);
    }
    else{
        subdir.replace("|","");
        // send(QString("{\n    \"copy\":\"%1\"\n}").arg(subdir).toUtf8(),1,index);
    }
}


void BusinessLogic::on_add_schedule(Schedule *schedule){
    schedule_list.append(schedule);
    QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();
//    emit /*scheduleUpdated*/(generateScheduleText());
    emit scheduleUpdated(serSchedule());
}


BusinessLogic::RSLT BusinessLogic::on_remove_schedule(int index){
    if(index<0||index>=schedule_list.size()){
        return Result(QString("index %1 out of range(0-%2)").arg(index).arg(schedule_list.size()));
    }
    schedule_list.removeAt(index);
    QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();
//    emit scheduleUpdated(generateScheduleText());
    emit scheduleUpdated(serSchedule());    
    return Result();
}


void BusinessLogic::on_suspended(){
    ninfo<<"应用程序退后台";
    // if(m_signalling){
    //     m_signalling->registerOffline();
    //     m_signalling->stop();
    // }
    m_rpepengine->destroy();
}


void BusinessLogic::on_resumed(){//此函数禁止使用。未绑定。
    ninfo<<"应用程序回到前台";
    // if(m_signalling){
    //     m_signalling->start();
    //     m_signalling->registerOnline();
    //     clients=m_signalling->getAllDevices();
    // }
    m_rpepengine->init();
}


void BusinessLogic::on_hangup_to_dfhn(){
    QSet<devid_t> l;
    for(auto i : clients){
        if(i.flag==Communication::DFHNDevice)
            l.insert(getIdByDevice(i));
    }
    if(l.empty())
//        emit messageBoxRequested("挂起","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN",MessageBoxType::Warning);
        emit businessEventOccurred(BusinessEvent::DFHNDeviceNotFound);
    else sendFile(l);
    //[this]{QList<device> l;for(auto i : clients){if(i.flag==Communication::DFHNDevice)l.append(i);}if(l.empty())QMessageBox::warning(this,"挂起","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN");else sendFile(l);});
    //[this]{QList<device> l;for(auto i : clients){if(i.flag==Communication::DFHNDevice)l.append(i);}if(l.empty())QMessageBox::warning(this,"下载","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN");else send("REQ_FILE",1,clients.indexOf(l[0]));});
}


void BusinessLogic::on_download_from_dfhn(){
    QSet<devid_t> l;
    for(auto i : clients){
        if(i.flag==Communication::DFHNDevice){
            // send("REQ_FILE",1,getIdByDevice(i));
            m_rpepengine->sendControl("REQ_FILE","",getIdByDevice(i));
            return;
        }
    }
//        emit messageBoxRequested("下载","当前设备列表中找不到DFHN设备。有关DFHN的更多信息，请参阅更多->帮助->DFHN",MessageBoxType::Warning);
        emit businessEventOccurred(BusinessEvent::DFHNDeviceNotFound);
}


void BusinessLogic::on_restart_all(){
    // m_transmissionengine->SPTP_sendCtrl("RESTART_NETWORK","",-2);
    for(auto i:clients.keys()){
        m_rpepengine->sendControl("RESTART_NETWORK","",i);
    }
}


RemoteControlEngine *BusinessLogic::getRemoteControlEngine(){
    return m_remotecontrolengine;
}


void BusinessLogic::on_start_remote(int index){
    ndb<<"开始远控";
    m_remotecontrolengine->startControl(index);
}


void BusinessLogic::on_stop_remote(){
    m_remotecontrolengine->stopControl();
}


device BusinessLogic::getPublicIp(){
    ndb<<"public_ip"<<public_ip;
    return public_ip;
}


void BusinessLogic::on_debug([[maybe_unused]]QVariant dbgArgs){
    
}


void BusinessLogic::on_readyRead(QByteArray msg){
    QJsonDocument jd = QJsonDocument::fromJson(msg);
    QJsonObject json;
    
    if(!jd.isObject()){
        if(!msg.startsWith("FB")&&!msg.startsWith("BF"))ninfo<<"var:msg = "<<msg;
        if(msg == "FILE_RELEASE_SUCCESSFULLY" && !chunks.empty()){
//            emit messageChanged(tr("文件发送可能成功"));
        }
        else if(msg == "REQ_FILE"){
            if(timer_is_uploading.isActive()){
                QEventLoop loop;
                connect(&timer_is_uploading,&QTimer::timeout,&loop,&QEventLoop::quit);
                loop.exec();
            }
            else QThread::msleep(500);
            auto keys = clients.keys();
            QMetaObject::invokeMethod(this,[=]{sendFile(QSet<devid_t>(keys.constBegin(),keys.constEnd()));},Qt::QueuedConnection);
        }
        else if(msg == "KEEP_ALIVE"){
            
        }
        else if(msg=="DING"){
            
        }
        else{
            ncritical<<"ERROR:msg isn`t an object!";
            ninfo<<QString(msg);
            return;
        }

    }
    else{
        json = jd.object();
    }
    
    //基本变量创建
    ipport sender = {
        json["ip"].toString(),
        static_cast<quint16>(json["port"].toInt())
    };
    int sender_index = getIdByDevice(sender);
    
    
    //消息解析
    if(json.contains("hole")){          //打洞
        QJsonObject replyJson;
        if(json["hole"].toInt() == 2){
//            replyJson.insert("hole",3);
//            send(QJsonDocument(json).toJson());
            // send("{\n    \"hole\":3\n}");
//            emit tempMessageChanged(QString("成功与%1建立连接").arg(sender),5000);
            emit businessEventOccurred(BusinessEvent::ConnectedSuccessfully,{{"ipport",sender.toString()}});
        }
        else if(json["hole"].toInt() == 3){
//            emit tempMessageChanged(QString("成功向%1进行NAT打洞").arg(sender),5000);
            emit businessEventOccurred(BusinessEvent::ConnectedSuccessfully,{{"ipport",sender.toString()}});
        }
        /*if(json["hole"].toInt() == 2){
            replyJson.insert("hole",3);
        }
        if(json["hole"].toInt() == 3     &&0){
            replyJson.insert("hole",4);
        }*/
    }
    if(json.contains("pat")){
//        ui->lineEdit_settings_githubPAT->setText(json["pat"].toString());
//        ui->pushButton_settings_save->click();
        RUN_LATER(
            on_settings_saved(user_name,pwd,mqtt_server.ip,mqtt_server.port,user_github_name,json["pat"].toString(),currentSkin,json_settings["use_log"].toBool(),json_settings["disable_notice"].toBool(),device_description);
        );
    }
    if(json.contains("cmd")){
#ifdef Q_OS_WIN
        QStringList trustList;trustList<<"shutdown";
        bool trust=0;
        for(auto j : trustList){
            if(json["cmd"].toString().startsWith(j))trust=1;
        }
//        if(!trust)attackProtection(at_influential,
//                                   QString("收到不信任的远程命令'%1'").arg(json["cmd"].toString()),
//                                   "没有执行这个命令",
//                                   []{});
        if(json["cmd"].toString().contains("shutdown")){is_accept_shutdown=true;emit businessEventOccurred(BusinessEvent::DestoryShutdownBlock);}
        if(trust)QProcess::startDetached(json["cmd"].toString());
#else
        // QMessageBox::information(this,"警告",QString(tr("警告：设备接收到来自远程设备'%1'的远程命令：\n\n%2\n\n这是Windows平台的特定命令，您的设备无法运行，已自动忽略。")).arg(sender).arg(json["cmd"].toString()));
#endif
    }
    if(json.contains("opt")){
        QString opt = json["opt"].toString();
        
        if(opt == "rtt_test"){
            // send("{\n    \"opt\":\"ack_rtt_test\"\n}",1,sender_index);
        }
        if(opt == "ack_rtt_test"){
            rtt_result[sender_index] = elapsed_rtt.elapsed();
        }
        if(opt == "start_remote"){
            timer_remote_sending.start(200);
            remote_be_device = sender;
//            send("{\n    \"opt\":\"start_remote_successfully\"\n}",1,sender_index);
        }
    }
    if(json.contains("get_folderList")){
        QFileInfoList infolist = QDir(json["get_folderList"].toString()).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks,QDir::DirsFirst|QDir::Name);
        QSet<QPair<bool,QString>> list;
        ninfo<<"size"<<infolist.size();
        std::cerr<<"size"<<infolist.size();
        QJsonObject json_reply;
        for(int ii=0;ii<infolist.size();ii++){
            auto i = infolist[ii];
            list.insert(QPair<bool,QString>(i.isFile(),i.fileName()));
            ninfo<<"扫描到"<<i.fileName();
        }
        QByteArray content;
        QDataStream stm(&content,QIODevice::WriteOnly);
        stm<<list.values();
        json_reply.insert("folderList",QString(content.toBase64()));
        json_reply.insert("folderList_folder",json["get_folderList"].toString());
        QThread::msleep(500);
        // send(QJsonDocument(json_reply).toJson(),1,sender_index);
    }
    if(json.contains("folderList")){
        QByteArray data = QByteArray::fromBase64(json["folderList"].toString().toUtf8());
        QDataStream stm(data);
        QList<QPair<bool, QString>> list;
        stm >> list;
//        dialog_remoteFile->setFileFolder(json["folderList_folder"].toString(),QSet<QPair<bool,QString>>(list.cbegin(),list.cend()));
        emit remoteFileFolderUpdated(json["folderList_folder"].toString(),QSet<QPair<bool,QString>>(list.cbegin(),list.cend()));
    }
    if(json.contains("copy")){
        QFile::copy(json["copy"].toString(),QDir("files/").absoluteFilePath(QFileInfo(json["copy"].toString()).fileName()));
    }
    if(json.contains("test_msg")){
//        QMessageBox::information(this,"收到了一个测试消息",(json["test_msg"].toString()));
        
//        send("DING",1,sender_index);
    }
    
}


void BusinessLogic::on_SPTP_readyRead(QByteArray msg){
    Utils::releaseFile(msg);
    RUN_IN_MAIN_THREAD(playSound(QUrl("qrc:/rc/audio/file_release_successfully.wav")););
}


void BusinessLogic::on_SPTP_ctrlMsg_received(TransmissionEngine::msg_ctrl msg){
    ninfo<<"收到控制消息 ctrl="<<msg.ctrl<<"value="<<msg.value;
    
    if(msg.ctrl=="RESTART_NETWORK"){
        
    }
}


void BusinessLogic::onControlReceived(QString key, QVariant value, devid_t src){
    if(key=="SHUTDOWN"){
        QProcess::startDetached("cmd.exe",{"/c","shutdown","/s","/t","10"});
    }
    if(key=="REQ_FILE"){
        sendFile({src});
    }
}


void BusinessLogic::onDataReceived(QByteArray data, devid_t src){
    Utils::releaseFile(data);
    RUN_IN_MAIN_THREAD(playSound(QUrl("qrc:/rc/audio/file_release_successfully.wav")););
}


//---------- 以下是私有函数 ----------


QByteArray BusinessLogic::encode(const QByteArray &msg){
    return Utils::encode(msg,pwd);
}


QByteArray BusinessLogic::decode(const QByteArray &msg){
    return Utils::decode(msg,pwd);
}


QStringList BusinessLogic::generateScheduleText(){
    QStringList ret;
    foreach(auto i,schedule_list){
        ret<<i->toString();
    }
    return ret;
}


QByteArray BusinessLogic::serSchedule(){
    QBuffer b;b.open(QBuffer::ReadWrite);
    QDataStream d(&b);
    foreach(auto i,schedule_list){
        d<<(*i);
    }
    b.seek(0);
    return b.readAll();
}


void BusinessLogic::unserSchedule(QByteArray dat){
    QBuffer b;b.open(QBuffer::ReadWrite);
    b.write(dat);
    QDataStream d(&b);
    foreach(auto i,schedule_list)i->deleteLater();
    schedule_list.clear();
    while(!d.atEnd()){
        Schedule *s = new Schedule(); 
        d>>(*s);
        schedule_list.append(s);
        connect(s,&Schedule::triggered,this,[this]{
            auto c=clients;c.remove(getIdByDevice(public_ip));
            auto m=c.keys();
            sendFile(QSet<devid_t>(m.constBegin(),m.constEnd()));
        });
    }
}


bool BusinessLogic::stat(){
    auto manager = new QNetworkAccessManager;
    auto func = [manager](QString url){
        QNetworkRequest request((QUrl(url)));
        auto reply = manager->get(request);
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        if(reply->error()!=QNetworkReply::NoError){
            ncritical<<"stat error:"<<reply->errorString();
        }
        return reply->error() == QNetworkReply::NoError;
    };
    
    bool succeeded1 = func("https://countapi.mileshilliard.com/api/v1/hit/nnpyro-synctunnel-stat-launchcnt");
    bool succeeded2 = true;
    if(is_first_launch){
        succeeded2 = func("https://countapi.mileshilliard.com/api/v1/hit/nnpyro-synctunnel-stat-downloadcnt");
    }
    
    manager->deleteLater();
    
    return succeeded1&&succeeded2;
}


void /*MainWindow::*/log(QtMsgType t, const QMessageLogContext &context, const QString &logstr){
    if(output_to_file){
//        QMutexLocker locker(&logFileMutex);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        QString opt;
        switch(t){
        case QtDebugMsg:opt="DEBUG";break;
        case QtInfoMsg:opt="INFO";break;
        case QtWarningMsg:opt="WARNING";break;
        case QtCriticalMsg:opt="CRITICAL";break;
        case QtFatalMsg:opt="FATAL";break;
        }
        
        QMetaObject::invokeMethod(logFile,[=]{
            if(!logFile->isOpen())logFile->open(QFile::WriteOnly);
            logFile->write(QString("[%1 %2]:%3\n\n").arg(time).arg(opt).arg(logstr).toLocal8Bit());
        });
        Q_UNUSED(context);
    }
    else{
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        QString color;
        switch(t) {
        case QtDebugMsg:
            color = "\033[2m";  // 灰色 (调试信息)
            break;
        case QtInfoMsg:
            color = "\033[0m";  // 绿色 (普通信息)
            break;
        case QtWarningMsg:
            color = "\033[33m";  // 黄色 (警告)
            break;
        case QtCriticalMsg:
            color = "\033[31m";  // 红色 (错误)
            break;
        case QtFatalMsg:
            color = "\033[1;41;37m";  // 白字红底加粗 (致命错误)
            break;
        default:
            color = "\033[0m";  // 默认重置颜色
            break;
        }
        std::cout<<QString("%1[%2]%3\033[0m").arg(color).arg(time).arg(logstr).toLocal8Bit().constData()<<std::endl;
    }
}

