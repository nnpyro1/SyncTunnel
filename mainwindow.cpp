#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "ui_dialog_deviceList.h"
#include <QFileInfoList>
#include <QDesktopServices>
#include <QProcess>
#include <QFile>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QStack>
#include <QThread>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QFileIconProvider>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#endif
#include <QScreen>
#include <QInputDialog>
#ifdef Q_OS_WIN
#include <winuser.h>
#include <dwmapi.h>
#endif
#include <QSysInfo>
#include <QUrlQuery>
#include <QSystemTrayIcon>
#include <QClipboard>
#include <QBuffer>
#include <QPainter>
#include <QUuid>
#include <QMetaEnum>
#include <qcachedbytearray.h>
#include <passport.h>
#include <QFileDialog>
#include <QShortcut>
#ifdef Q_OS_ANDROID
// #include <QAndroidJniEnvironment>
// #include <QAndroidJniObject>
// #include <QtAndroid>
#endif

//#define process_events_without_useript QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents)
////#define idToString(id) #id

////调试定义
////#define DEBUG_NO_ENCRYPTION

//using namespace std;


//MainWindow::MainWindow(BusinessLogic *bl,QWidget *parent, std::function<void(QString)> func_update, bool bShow)
//    : QMainWindow(parent)
//    , businesslogic(bl)
//    , ui(new Ui::MainWindow)
//    , settings("config/settings.ini",QSettings::IniFormat)
//{
//    //对象创建
//    m_communication = new Communication;
//    m_signalling = new Signalling;
//    m_storage = new Storage;
//    m_transmissionengine = nullptr;
//    label_status = new QLabel(this);
//    process_proxy = new QProcess(this);
//    process_proxy_ui = new QProcess(this);
//    line_ackloop = new QLineSeries(this);
//    line_delay = new QLineSeries(this);
//    line_speed = new QLineSeries(this);
////    dialog_remoteFile = new Dialog_remoteFile(this);
//    logFile = new QFile(QString("logs/%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")).toStdString().c_str(),this);
    
//    //安卓平台设置
//#ifdef Q_OS_ANDROID
//    /*QString currentWorkPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//    ninfo<<"工作目录信息配置正确。"<<currentWorkPath;
//    if(!currentWorkPath.contains("Android/data")){
//        ninfo<<"工作目录:"<<currentWorkPath;
//        QString ph = "/storage/emulated/0/Android/data/com.nnpyro.SyncTunnel/";
////        ui->statusBar->showMessage("成功配置了工作目录",1);
//        QDir dir(ph);
//        if(!dir.exists())dir.mkpath(".");
//        QDir::setCurrent(ph);
//    }*/
//    QAndroidJniObject context = QtAndroid::androidContext();
//    if(!context.isValid()){
//        ncritical<<"context 无效";
//        QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\ncontext无效"));
//    }
//    else{
//        QAndroidJniObject fileObj = context.callObjectMethod("getExternalFilesDir","(Ljava/lang/String;)Ljava/io/File;",nullptr);
//        if(!fileObj.isValid()){
//            ncritical<<"fileObj 无效";
//            QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\nfileObj无效"));
//        }
//        else{
//            QAndroidJniObject o = fileObj.callObjectMethod("getAbsolutePath","()Ljava/lang/String;");
//            ninfo<<"path:"<<o.toString();
//            QString path = o.toString();
//            if(path.isEmpty()){
//                ncritical<<"path 无效";
//                QMessageBox::critical(this,tr("SyncTunnel 错误"),tr("无法设置运行目录。将使用私有目录，功能受限。\npath无效"));
//            }
//            else{
//                QDir(path).mkpath(".");
//                QDir::setCurrent(path);
//                ninfo<<"当前目录："<<QDir::currentPath();
//            }
//        }
//    }
//#endif
    
//    //基本设置
//    ui->setupUi(this);
//    resize(QGuiApplication::primaryScreen()->geometry().width()*0.45,QGuiApplication::primaryScreen()->geometry().height()*0.55);
//    setWindowTitle("SyncTunnel 同步隧道");
//    ui->tabWidget->setCurrentIndex(5);
//    timer_is_uploading.setSingleShot(true);
//    timer_clear_currentFileMap.setSingleShot(true);
//    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置自动列宽
//    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);//设置自动列宽
//    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);//设置自动列宽
//    setAcceptDrops(true);//接收拖放
//    ui->listWidget_file->setAcceptDrops(true);// 允许接收拖放事件
//    ui->listWidget_file->setDragDropMode(QAbstractItemView::NoDragDrop);//让事件传递到MainWindow
//#ifdef Q_OS_WIN
//    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);//设置优先级为高
//#endif
//    {//添加Item   必需和枚举的一一对应
//        ui->comboBox_settings_uiskin->addItem(tr("科技蓝（默认）"),Dark);
//        ui->comboBox_settings_uiskin->addItem(tr("纯洁白"),Light);
//        ui->comboBox_settings_uiskin->addItem(tr("银色星辰（限定版，前50个下载本软件的用户可申请）"),Silver);
//        ui->comboBox_settings_uiskin->addItem(tr("金色流光（限定版，对本软件的创作有突出贡献的用户可申请）"),Golden);
//    }
////    ui->tableWidget_deviceList.
//    setWindowIcon(QIcon(":/rc/img/favicon.ico"));
//    trayIcon->setIcon(QIcon(":/rc/img/favicon.ico")); 
//    trayIcon->setContextMenu(new QMenu(this));
//    trayIcon->contextMenu()->addAction(ui->actionExit_Application);
//    ui->label_remote_screen->installEventFilter(this);
//    line_ackloop->setColor(QColor(Qt::green));
//    line_speed->setColor(QColor(Qt::red));
//    line_delay->setColor(QColor(Qt::yellow));
//    line_ackloop->setName("检验轮个数");
//    line_delay->setName("发送延迟");
//    line_speed->setName("发送速度");
//    chart_send->addSeries(line_speed);
//    chart_send->addSeries(line_ackloop);
//    chart_send->addSeries(line_delay);
//    chart_send->addAxis(axis_x,Qt::AlignBottom);
//    chart_send->addAxis(axis_y,Qt::AlignLeft);
//    chart_send->addAxis(axis_y_r,Qt::AlignRight);
//    ui->widget_info_chart1->setChart(chart_send);
//    line_speed->attachAxis(axis_x);line_speed->attachAxis(axis_y_r);
//    line_ackloop->attachAxis(axis_x);line_ackloop->attachAxis(axis_y);
//    line_delay->attachAxis(axis_x);line_delay->attachAxis(axis_y);
//    //隐藏不必要的标签页
//    restartDebug();
//    hideTab(ui->tabWidget,2);
//    hideTab(ui->tabWidget,3);
//    hideTab(ui->tabWidget,5);
//    hideTab(ui->tabWidget,6);
//    ui->tabWidget->tabBar()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//    //取消鼠标滚轮
//    ui->spinBox_settings_mqttPort->installEventFilter(this);
//    ui->comboBox_settings_uiskin->installEventFilter(this);
//    ui->comboBox_settings_language->installEventFilter(this);
//#ifdef NNPYRO_USE_CONSOLE//使用控制台
//    if(!QApplication::arguments().contains("CON_MODE")){if(!QDir("tools/").exists())QDir().mkpath("tools/");QFile::copy(":/rc/bin/Alacritty.exe","tools/Alacritty.exe");QProcess::startDetached("tools/Alacritty.exe",QStringList()<<"-e"<<QApplication::applicationFilePath()<<QApplication::arguments()<<"CON_MODE");close();QApplication::quit();}
//#endif
//#ifdef NNPYRO_COLORFULCON//使用控制台
//    qInstallMessageHandler(static_cast<QtMessageHandler>(log));
//#endif
    
//    //读取文件中的用户名密码
//    QFile file1("config/1.nprivate0");
//    file1.open(QIODevice::ReadOnly);
//    QTextStream user_config_stream1(&file1);
//    user_name = QByteArray::fromBase64(user_config_stream1.readLine().toUtf8());
//    auto encryptedPwd=(QByteArray::fromBase64(user_config_stream1.readLine().toUtf8()));
//    auto iv = encryptedPwd.mid(0,16);
//    pwd = QAESEncryption::RemovePadding(QAESEncryption::Decrypt(QAESEncryption::AES_256,QAESEncryption::CBC,encryptedPwd.mid(16),SYNCTUNNEL_CRPT_KEY,iv));
//    if(user_name.isEmpty())user_name="DefaultUser";
//    ui->lineEdit_settings_username->setText(user_name);
//    ui->lineEdit_settings_pwd->setText(pwd);
//    ui->statusBar->addPermanentWidget(label_status);
//    //读取文件中的mqtt服务器设置
//    QFile file2("config/2.nprivate0");
//    file2.open(QIODevice::ReadOnly);
//    QTextStream user_config_stream2(&file2);
//    mqtt_server = ipport{
//            QString(QByteArray::fromBase64(user_config_stream2.readLine().toUtf8())),
//            static_cast<quint16>(user_config_stream2.readLine().toInt())
//    };
//    if(mqtt_server.ip.isEmpty()){
//        mqtt_server={"broker.emqx.io",1883};
//    }
//    ui->lineEdit_settings_mqttServer->setText(mqtt_server.ip);
//    ui->spinBox_settings_mqttPort->setValue(mqtt_server.port);
//    //读取文件中的Github服务器设置
//    QFile file3("config/3.nprivate0");
//    file3.open(QIODevice::ReadOnly);
//    QTextStream user_config_stream3(&file3);
//    user_github_name = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
//    user_github_PAT = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
//    ui->lineEdit_settings_gitubUser->setText(user_github_name);
//    ui->lineEdit_settings_githubPAT->setText(user_github_PAT);
//    //读取文件中应用程序配置
//    QFile file4("config/4.nprivate0");
//    file4.open(QIODevice::ReadOnly);
//    QTextStream user_config_stream4(&file4);
//    skinType skin = (skinType)user_config_stream4.readLine().toInt();
//    QFile file_QSS;
//    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
//    switch(skin){
//    case Dark:
//        currentSkin=skin;
//        file_QSS.setFileName(":/rc/style/dark.qss");
//        file_QSS.open(QFile::ReadOnly);
//        setStyleSheet("");
//        app->setStyleSheet(file_QSS.readAll());
//        break;
//    case Light:
//        currentSkin=skin;
//        file_QSS.setFileName(":/rc/style/light.qss");
//        file_QSS.open(QFile::ReadOnly);
//        setStyleSheet("");
//        app->setStyleSheet(file_QSS.readAll());
//        break;
//    case Golden:
//        if(checkSkin(Golden)){
//            currentSkin=skin;
//            file_QSS.setFileName(":/rc/style/golden.qss");
//            file_QSS.open(QFile::ReadOnly);
//            setStyleSheet("");
//            app->setStyleSheet(file_QSS.readAll());
//            setWindowTitle(tr("SyncTunnel 同步隧道（金色流光限定版） 感谢您本对软件做出的贡献"));
//        }
//        else{
//            QMessageBox::information(this,tr("SyncTunnel"),tr("您无法使用限量版UI。"));
//        }
//        break;
//    case Silver:
//        if(checkSkin(Silver)){
//            currentSkin=skin;
//            file_QSS.setFileName(":/rc/style/silver.qss");
//            file_QSS.open(QFile::ReadOnly);
//            setStyleSheet("");
//            app->setStyleSheet(file_QSS.readAll());
//            setWindowTitle("SyncTunnel 同步隧道（银色星辰限定版）");
//        }
//        else{
//            QMessageBox::information(this,"SyncTunnel","您无法使用限量版UI。");
//        }
//        break;
//    }
//    ui->comboBox_settings_uiskin->setCurrentIndex(currentSkin);
//    ninfo<<"current Skin="<<currentSkin;
////    读取JSON配置
//    QFile file_json("config/config.json");
//    file_json.open(QFile::ReadOnly);
//    json_settings = QJsonDocument::fromJson(file_json.readAll()).object();
//    file_json.close();
//    if(json_settings["use_log"].toBool()){
//        if(!QDir("logs").exists())QDir("logs").mkpath(".");
//        logFile->open(QFile::WriteOnly);
//        output_to_file = true;
//        qInstallMessageHandler(static_cast<QtMessageHandler>(log));
//        ninfo<<"日志输出重定向到文件";
//        ui->checkBox_settings_recordLog->setCheckState(Qt::Checked);
//    }
//    if(json_settings["disable_notice"].toBool())ui->checkBox_settings_disableNotice->setCheckState(Qt::Checked);
//    device_description=json_settings["description"].toString();
////    device_flag=json_settings["device_flag"].toInt();
//    ui->lineEdit_settings_description->setText(device_description);
//    if(settings.contains("ApplicationSettings/syncSourceDir")){
//        syncFolder=QDir(settings.value("ApplicationSettings/syncSourceDir").toString());
//    }
//    else{
//        syncFolder=QDir("files/");
//    }
//    is_autoSync=settings.value("ApplicationSettings/isAutoSync").toBool();
//    ui->checkBox_file_autoSync->setCheckState(is_autoSync?Qt::Checked:Qt::Unchecked);
//    //读取Schedule日程配置
//    QFile file_schedule("config/schedule.dat");
//    file_schedule.open(QFile::ReadOnly);
//    QDataStream d(&file_schedule);
//    while(!d.atEnd()){
//        Schedule *s = new Schedule(this); 
//        d>>(*s);
//        schedule_list.append(s);
//        ui->listWidget_schedule->addItem(s->toString());
//    }
//    file_schedule.close();

//    //设置当前页
//    QDir dir_empty_label1("config/empty/label1");
//    if(dir_empty_label1.exists()){
//        ui->tabWidget->setCurrentIndex(0);
//        is_first_launch = false;
//    }
//    else{
//        dir_empty_label1.mkpath(".");
//        ui->tabWidget->setCurrentIndex(5);
//        is_first_launch = true;
//    }
//    QDir dir_emty_label2("config/empty/label2");//不启用关机阻止
//    if(!dir_emty_label2.exists()){
//#ifdef Q_OS_WIN
//        ShutdownBlockReasonCreate((HWND)winId(),L"如果想要离线非挂起高速无限同步文件请不要关机 SyncTunnel 离线文件同步服务");
//#endif
//    }
//    QDir dir_empty_label3("config/empty/label3");//DFHN客户端
//    if(dir_empty_label3.exists()){
//        device_flag = Communication::DFHNDevice;
//        is_DFHN = true;
//    }
//    else{
//        device_flag = Communication::WindowsDevice;
//        winRun device_flag = Communication::WindowsDevice;
//        androidRun device_flag = Communication::AndroidDevice;
//        linuxRun device_flag = Communication::LinuxDevice;
//        is_DFHN = false;
//    }
    
    
//    //信号槽绑定
//    connect(ui->listWidget_file,&QListWidget::doubleClicked,this,&MainWindow::on_folder_change);
//    connect(ui->listWidget_file,&QListWidget::customContextMenuRequested,this,&MainWindow::on_rightclick);
//    connect(ui->actionOpen,&QAction::triggered,this,[this]{
////        ndb<<QStringList(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath());
////        QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath())));
//        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath()));
//    });
//    connect(ui->actionFolder,&QAction::triggered,this,[this]{/*QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators((current_dir).absolutePath())));*/QDesktopServices::openUrl(QUrl::fromLocalFile(current_dir.absolutePath()));});
////    connect(m_communication,&Communication::readyRead,this,&MainWindow::on_readyRead);
////    if(1)connect(m_signalling,&Signalling::on_userlist_updata,this,[this](QList<Communication::ipport> userl){qDebug()<<"信号触发";ui->textBrowser_debug1->clear();foreach(auto i,userl)ui->textBrowser_debug1->append(i);});
////    connect(&timer_fileResend,&QTimer::timeout,this,&MainWindow::on_request_resend); 
////    connect(&timer_is_uploading,&QTimer::timeout,this,[this]{chunks.clear();is_uploading=false;send_current_delay=SEND_MAX_DELAY-10;send_stable_count=0;send_ack_count.clear();send_req_ack_loop=5;send_lost_loop_count=0;send_lost_count.clear();foreach(auto i,schedule_list)i->setEnabled(true);});      //发送方清除状态
//    connect(ui->pushButton_settings_save,&QPushButton::clicked,this,&MainWindow::on_settings_saved);
//    connect(ui->actionupload_file,&QAction::triggered,this,[this]{sendFile();});
////    connect(&timer_clear_currentFileMap,&QTimer::xtimeout,this,[this]{currentFileMap.clear();currentFileTotal = -1;receive_lost_count=0;receive_last_pack_index=-1;receive_last_ack_total=-1;timer_fileResend.stop();receive_last_ack_index=-1;foreach(auto i,schedule_list)i->setEnabled(true);});//接收方清除状态
//    connect(ui->actionHangup,&QAction::triggered,this,&MainWindow::on_hangup);
//    connect(ui->actionDownload,&QAction::triggered,this,&MainWindow::on_download);
//    connect(ui->actionSync_PAT,&QAction::triggered,this,[this]{
//        QJsonObject json;json.insert("pat",ui->lineEdit_settings_githubPAT->text());
//        send(QJsonDocument(json).toJson());
//    });
//    connect(ui->checkBox_settings_ipv6,&QCheckBox::clicked,this,[this](bool isCheck){
//        if(isCheck)QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments()<<("-ipv6"));
//        else{
//            QStringList l=QApplication::arguments();l.removeAll("-ipv6");
//            QProcess::startDetached(QApplication::applicationFilePath(),l);
//        }close();
//    });
//    connect(ui->actionRefresh,&QAction::triggered,this,[this]{show_dir();});
//    connect(ui->pushButton_switchProxy,&QPushButton::clicked,this,&MainWindow::on_proxy);
//    connect(process_proxy,&QProcess::readyRead,this,[this]{ui->textBrowser_proxy->append(QString::fromLocal8Bit(process_proxy->readAll()));});
////    connect(ui->tableWidget_deviceList,&QTableWidget::doubleClicked,this,)
//    connect(&timer_savePower,&QTimer::timeout,this,[this]{
//        label_status->setText(QString(tr("SyncTunnel正在等待文件传输请求……\n如果没有请求，计算机将会在%1秒后关闭\n您也可以用Alt+F4关闭此窗口后手动关机")).arg(/*QTime().addMSecs(timer_savePower_finish.remainingTime()).toString("HH时mm分ss秒"))*/timer_savePower_finish.remainingTime()/1000));
//    });
//#ifdef Q_OS_WIN
//    connect(&timer_savePower_finish,&QTimer::timeout,this,[this]{
//        ShutdownBlockReasonDestroy((HWND)winId());
//        QProcess::startDetached("shutdown",{"-s","-t","10"});(void)this;
//    });
//    connect(ui->actionShutdown,&QAction::triggered,this,[this]{ShutdownBlockReasonDestroy((HWND)winId());is_accept_shutdown=true;QProcess::startDetached("shutdown",{"-s","-t","10"});});
//#endif
//    connect(ui->actionShutdown_current,&QAction::triggered,this,[this]{
//        int index = ui->tableWidget_deviceList->currentRow();
//        if(index==-1){
//            QMessageBox::information(this,tr("关闭选中的设备"),tr("请先选中一个设备！"));return;
//        }
//        m_communication->send(clients[index],encode("{\n    \"cmd\":\"shutdown -s -t 10\"\n}"));
//    }); 
//    connect(ui->tableWidget_deviceList,&QTableWidget::customContextMenuRequested,this,&MainWindow::on_rightclick_deviceList);  
//    connect(ui->actionTest_RTT,&QAction::triggered,this,&MainWindow::on_test_rtt);
//    connect(ui->pushButton_settings_requestUI,&QPushButton::clicked,this,[]{
//        QUrl url("mailto:nnpyro2@outlook.com");
//        QUrlQuery query;
//        query.addQueryItem("subject","SyncTunnel申请限定款UI");
//        query.addQueryItem("body",QString("您好！\n我是SyncTunnel的用户，我想要申请(填写您需要申请的界面样式 金色流光/银色星辰)UI样式。\n\n我对软件开发创作做出了如下突出贡献\n（请填写此处。或者您可以填写您是前50位下载本软件的用户）\n\n我的设备唯一ID：\n%1").arg(QString(QSysInfo::machineUniqueId())));
//        url.setQuery(query);QDesktopServices::openUrl(url);
//    });
//    connect(trayIcon,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason reason){Q_UNUSED(reason);if(isMinimized())showNormal();if(isHidden())show();raise();});
//    connect(ui->pushButton_copyId,&QPushButton::clicked,this,[this]{QApplication::clipboard()->setText(QSysInfo::machineUniqueId());QMessageBox::information(this,"SyncTunnel","复制成功！");});
//    connect(ui->actionRequestFile,&QAction::triggered,this,[this]{if(ui->tableWidget_deviceList->currentRow()==-1){QMessageBox::warning(this,"错误","请先选中一个设备");return;} send("REQ_FILE",1,ui->tableWidget_deviceList->currentRow());label_status->setText("正在等待发送方响应...");});
//    connect(&timer_keepAlive,&QTimer::timeout,this,[this]{if(chunks.empty()&&currentFileMap.empty())send("KEEP_ALIVE");});
////    connect(dialog_remoteFile,&Dialog_remoteFile::send,this,[this](QByteArray data,bool b,int e){send(data,b,e);});
////    connect(ui->actionRemoteCopyFile,&QAction::triggered,this,[this]{if(ui->tableWidget_deviceList->currentRow()==-1){QMessageBox::warning(this,"错误","你需要先选中一个设备");return;}copy_remote_file(ui->tableWidget_deviceList->currentRow()});
//    connect(ui->actionRemoteCopyFile, &QAction::triggered, this, [this] {int currentRow = ui->tableWidget_deviceList->currentRow();if (currentRow == -1) {QMessageBox::warning(this, "错误","你需要先选中一个设备");return;}copy_remote_file(currentRow);});
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
            
////        QThread::msleep(40);send("REMOTE_IMG_1_"+a1,1,n);
////        QThread::msleep(30);send("REMOTE_IMG_2_"+a2,1,n); 
////        QThread::msleep(30);send("REMOTE_IMG_3_"+a3,1,n); 
////        QThread::msleep(30);send("REMOTE_IMG_4_"+a4,1,n);
//        ndb<<"发送屏幕截图";
        
//    });
//    connect(ui->pushButton_remote_stop,&QPushButton::clicked,this,[this]{send("STOP_REMOTE",1,clients.indexOf(remote_device));});
//    connect(ui->pushButton_remote_left,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mleft\"\n}");});
//    connect(ui->pushButton_remote_right,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mright\"\n}");});
//    connect(ui->pushButton_remote_up,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mup\"\n}");});
//    connect(ui->pushButton_remote_down,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mdown\"\n}");});
//    connect(ui->pushButton_remote_mid,&QPushButton::clicked,this,[this]{send("{\n    \"remote_event\":\"mmid\"\n}");});
//    connect(ui->actionOpen_DriveCrypto,&QAction::triggered,this,[]{QProcess::startDetached("DriveCrypto.exe");});
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
//    connect(shortcut_debug,&QShortcut::activated,this,[this]{is_debug=is_debug?false:true;restartDebug();QMessageBox::information(this,"debug",QString("您已%1调试模式").arg(is_debug?"进入":"离开"));});
//    connect(ui->checkBox_info_viewLowDelay,&QCheckBox::clicked,this,[this]{if(ui->checkBox_info_viewLowDelay->isChecked())axis_y->setRange(0,100);else axis_y->setRange(0,SEND_MAX_DELAY);});
//    connect(ui->pushButton_settings_recordLog,&QPushButton::pressed,this,[this]{if(!QDir("logs").exists())QDir("logs").mkpath(".");logFile->open(QFile::WriteOnly);ninfo<<"类"<<this<<"将日志输出重定向到文件";output_to_file = true;qInstallMessageHandler(static_cast<QtMessageHandler>(log));ninfo<<"日志输出重定向成功！";});
//    connect(ui->actionAdd_file,&QAction::triggered,this,[this]{QString dir=QFileDialog::getOpenFileName(this);QFile(dir).copy(current_dir.absolutePath());});
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
//                clients = m_signalling->getUserList();}}});
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
//    connect(&timer_autoSync,&QTimer::timeout,this,[this]{
//        if(!is_autoSync)return;
//        auto newFileHashMap = generateFileHashMap(syncFolder);
//        //比较
//        bool flag=false;
//        for(auto it = newFileHashMap.constBegin();it!=newFileHashMap.end();it++){
//            if (!fileHashMap.contains(it.key()) || it.value() != fileHashMap.value(it.key())) {
//                flag = true;
//                break;
//            }
//        }
//        if(flag==true){
//            ninfo<<"检测到文件修改，自动同步";
//            fileHashMap=newFileHashMap;
//            sendFile(lastSyncDst);
//        }
//        else{
            
//        }
//    });
//    connect(ui->actionRestart_all_applications,&QAction::triggered,this,[this]{m_transmissionengine->SPTP_sendCtrl("RESTART_NETWORK","",-2);});
    
//    //目录显示
//    current_dir = QDir("files/");
//    show_dir();
    
    
//    initNetwork(func_update);
    
    
//    //参数处理
//    ninfo<<"Application arguments:"<<QApplication::arguments();
//    auto args = QApplication::arguments();
    
//    if(args.contains("-proxy")){//加速
//        on_proxy();
//    }
//    if(args.contains("-ipv6")){//强制使用ipv6
//        ui->checkBox_settings_ipv6->setChecked(true);
//    }
//    if(args.contains("CON_MODE")){
//        output_to_file=false;
//        qInstallMessageHandler(static_cast<QtMessageHandler>(log));
//    }
    
//    //其他部分
//    trayIcon->show();
//    timer_keepAlive.start(15000);
//    timer_autoSync.start(10000);//每隔10秒刷新
//    winRun if(!json_settings["disable_notice"].toBool())QProcess::startDetached("Noticer.exe");
    
//    QTimer::singleShot(1000,this,[this]{raise();});
//    if(is_first_launch){
//        QMetaObject::invokeMethod(this,[this]{
//            int button1 = QMessageBox::information(this,"向导","为了节省您宝贵的时间，我们可以为您展示一个快速配置向导，引导您快速配置软件。请问您需要吗？",QMessageBox::Yes|QMessageBox::No);
//            if(button1==QMessageBox::Yes){
//                QString qss = qApp->styleSheet();
//                qApp->setStyleSheet("");
//                Wizard_startup *w = new Wizard_startup(this);
//                connect(w,&Wizard_startup::settingsSaved,this,[this](QString username,QString pwd,QString gh,QString ghpat){
//                    ui->lineEdit_settings_username->setText(username);
//                    ui->lineEdit_settings_pwd->setText(pwd);
//                    ui->lineEdit_settings_githubPAT->setText(ghpat);
//                    ui->lineEdit_settings_gitubUser->setText(gh);
//                    ui->pushButton_settings_save->click();
//                });
//                w->exec();
//                qApp->setStyleSheet(qss);
//            }
//        },Qt::QueuedConnection);
//    }
//    if(!QDir("files/").exists())QDir("files").mkpath(".");
    
//    timer_refresh.start(2500);
//    androidComp( QTimer::singleShot(10,this,[this]{
//        setMaximumSize(QApplication::primaryScreen()->availableGeometry().size());/*QString str;QDebug(&str)<<"maxsize"<<maximumSize()<<"DPI"<< QGuiApplication::primaryScreen()->physicalDotsPerInch();QMessageBox::information(this,"",str);*/
//        ui->centralwidget->setMaximumSize(maximumSize());
//        ui->tabWidget->setMaximumSize(maximumSize());
//        ui->centralwidget->resize(maximumSize());
//        ui->tabWidget->resize(maximumSize());
//        int width = ui->tabWidget->size().width()/4;
//        //            ndb<<"width"<<width;
//        //            ninfo<<width;
//        ui->tabWidget->tabBar()->setStyleSheet(/*ui->tabWidget->tabBar()->styleSheet()+*/QString("QTabBar::tab:enabled{ width: %1px; margin: 0px; padding: 0px; }").arg(width));
//        ui->tabWidget->tabBar()->setExpanding(true);
//        ui->tabWidget->setTabPosition(QTabWidget::South);
//        hideTab(ui->tabWidget,2);
//        hideTab(ui->tabWidget,3);
//        hideTab(ui->tabWidget,5);
//        hideTab(ui->tabWidget,6);
//    }); )
//    fileHashMap=generateFileHashMap(syncFolder);
//    ndb<<syncFolder.absolutePath();
//}


//MainWindow::~MainWindow(){
//    m_communication->deleteLater();
//    m_signalling->deleteLater();
//    m_storage->deleteLater();
//    m_transmissionengine->deleteLater();
//    delete ui;
//    delete logFile;
//}


//void MainWindow::send(QByteArray msg, bool e, int d){
//    /*//自动补全信息
//    QJsonDocument jd = QJsonDocument::fromJson(msg);
//    if(jd.isObject()){
//        QJsonObject json = jd.object();
//        if(!json.contains("user")){
//            json.insert("user",user_name);
//        }
//        if(!json.contains("ip")){
//            json.insert("ip",public_ip.ip);
//        }
//        if(!json.contains("port")){
//            json.insert("port",public_ip.port);
//        }
//        msg = QJsonDocument(json).toJson();
//    }
    
//    //加密并发送
//    auto cli = clients;
//    cli.removeAll(public_ip);//不给自己发送
//    if(d==-1)foreach(auto i,cli)m_communication->send(i,e?encode(msg):msg);
//    else m_communication->send(clients[d],e?encode(msg):msg);*/
//    m_transmissionengine->send(msg,e,d);    
//}


//QByteArray MainWindow::encode(QByteArray msg){
//#ifndef DEBUG_NO_ENCRYPTION
//    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
//    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
//    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
//    QByteArray encode = encription.encode(msg,key,iv)/*.toBase64()*/;
//    return /*qCompress(*/encode/*,9)*/;
//#else   
//    return msg;
//#endif
//}


//QByteArray MainWindow::decode(QByteArray msg){
//#ifndef DEBUG_NO_ENCRYPTION
//    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
//    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
//    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
//    QByteArray decoded = encription.removePadding(encription.decode(/*QByteArray::fromBase64(msg)*//*qUncompress(*/msg/*)*/,key,iv));
//    return decoded;
//#else
//    return msg;
//#endif
//}



//QByteArray MainWindow::mergeFile(QDir folder, bool c){
//    /*bool sync_all = incremental_sync_set.empty();
//    QByteArray f;
//    QFileInfoList info = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsLast);
//    ninfo<<"进入目录"<<folder.absolutePath()<<"长度"<<info.size();
//    foreach(QFileInfo fi ,info){
//        if(fi.isFile()){
//            ninfo<<"处理文件"<<fi.absoluteFilePath();
////            ndb<<"文件标识："<<QDir("files").relativeFilePath(fi.absoluteFilePath());
//            if(sync_all || incremental_sync_set.contains(QDir("files/").relativeFilePath(fi.filePath()))){
//                f += "FILE\n";
//    //            QString filepath = fi.canonicalFilePath();
//    //            QString dirpath = QDir("files/").canonicalPath();
//    //            f += filepath.mid(dirpath.size()+1) + "\n";
//                QString absPath = fi.canonicalFilePath();
//                QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
//                f += relativePath + "\n";
//                QFile file(absPath);
//                file.open(QIODevice::ReadOnly);
//    //            f += qCompress(file.readAll(),9).toBase64() + "\n";
//                QByteArray data = qCompress(file.readAll(),9);
//                f += QString::number(data.size()) + "\n";
//                if(c)f += data;else f += "[FILE_CONTENTS_HERE]\n";
//            }
//            else{
//                ninfo<<"忽略用户不同步的文件"<<fi.absoluteFilePath();
//            }
//        }
//        else{//目录递归DFS
//            ninfo<<"处理目录"<<fi.absoluteFilePath();
//            f += "DIR\n";
//            QString absPath = fi.canonicalFilePath();
//            QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
//            f += relativePath + "\n";
////            f += mergeFile(absPath);
//            f += mergeFile(QDir(relativePath),c);
//        }
//    }
//    return f;*/
//    QByteArray f;
//    auto fil = traverseFolder(folder);
//    bool sync_all = incremental_sync_set.empty();
//    foreach(auto fim , fil){
//        auto fi=fim.first;
//        if(fi.isFile()){
//            ninfo<<"处理文件"<<fi.absoluteFilePath();
////            ndb<<"文件标识："<<QDir("files").relativeFilePath(fi.absoluteFilePath());
//            if(sync_all || incremental_sync_set.contains(QDir("files/").relativeFilePath(fi.filePath()))){
//                f += "FILE\n";
//    //            QString filepath = fi.canonicalFilePath();
//    //            QString dirpath = QDir("files/").canonicalPath();
//    //            f += filepath.mid(dirpath.size()+1) + "\n";
//                QString absPath = fi.canonicalFilePath();
//                QString relativePath = QDir("files/../").relativeFilePath(fim.second.absolutePath());
//                f += relativePath + "\n";
//                QFile file(absPath);
//                file.open(QIODevice::ReadOnly);
//    //            f += qCompress(file.readAll(),9).toBase64() + "\n";
//                QByteArray data = qCompress(file.readAll(),9);
//                f += QString::number(data.size()) + "\n";
//                if(c)f += data;else f += "[FILE_CONTENTS_HERE]\n";
//            }
//            else{
//                ninfo<<"忽略用户不同步的文件"<<fi.absoluteFilePath();
//            }
//        }
//        else{//无需目录递归DFS
//            ninfo<<"处理目录"<<fi.absoluteFilePath();
//            f += "DIR\n";
//            QString absPath = fi.canonicalFilePath();
//            QString relativePath = QDir("files/../").relativeFilePath(fi.absoluteFilePath());
//            f += relativePath + "\n";
////            f += mergeFile(absPath);
////            f += mergeFile(QDir(relativePath),c);
//        }
//    }
//    return f;
//}

//void MainWindow::sendFileTo(int n)
//{
//    m_transmissionengine->SPTP_sendTo(n,mergeFile(QDir("files/")));
//}



///*void MainWindow::sendFileTo(int n){
//    //发送前准备
//    foreach(auto s , schedule_list){//禁用日程
//        s->setEnabled(false);
//    }
//    currentSendDst = n;
//    //进行客户端连通性测试
//    test_if_connected_set.clear();
//    label_status->setText(tr("正在测试连通性"));
//    send("{\n    \"opt\":\"test_if_connected\"\n}",1,currentSendDst);
//    QEventLoop loop_test;
//    connect(this,&MainWindow::signal_test_if_connected_finished,&loop_test,&QEventLoop::quit);
//    QTimer::singleShot(5000,&loop_test,&QEventLoop::quit);
//    bool is_test_success = false;
//    connect(this,&MainWindow::signal_test_if_connected_finished,this,[&,this]{is_test_success=true;(void)this;});
//    loop_test.exec();
//    ninfo<<"var:is_test_success="<<is_test_success;
//    if(!is_test_success){
//        QSet<ipport> missing = QSet<ipport>(clients.begin(),clients.end()) - test_if_connected_set;
//        QStringList missingStr;
//        foreach(auto i , missing){missingStr<<i;}
//        int button = QMessageBox::warning(this,tr("连通性警告"),QString(tr("以下客户端没有响应连通性测试:\n\n%1\n\n这可能是由于客户端掉线造成的，也可能是网络波动。\n如果您想继续发送，请点击“是”。如果您想取消发送，请点击“否”。如果您想重试，请点击“重试”。")).arg(missingStr.join("\n")),QMessageBox::Yes|QMessageBox::No|QMessageBox::Retry);
//        switch (button) {
//        case QMessageBox::Yes:
//            break;
//        case QMessageBox::Retry:
//            sendFileTo(n);//自动到下面的return
//        case QMessageBox::No:
//            return;
//        }
//    }
    
    
//    //生成文件表
//    is_uploading = true;
//    label_status->setText(tr("正在加载文件……"));
//    process_events_without_useript;
//    QByteArray fileList = mergeFile((QDir)"files/");
//    ui->textBrowser_debug1->append(QString::number(fileList.size()));
//    chunks.clear();
    
//    //文件分片
////    const int SPC = /*3 * 1024;//3kb*/ /*1400;//基于MTU1500的值*/ /*1024;//小值* / 8 * 1024 -1;//7kb
////    const int SPC = 1382;//基于MTU1500的值
//    const int SPC = 1420;
//    label_status->setText(tr("正在分片文件……"));
//    process_events_without_useript;
////    QTextStream stm(&fileList);
////    for(;!stm.atEnd();){
////        chunks.append(stm.read(SPC));
////    }
//    for(int i=0;i<fileList.size();i+=SPC){
//        chunks.append(fileList.mid(i,SPC));
//    }
    
    
////    QJsonObject json;
//    QList<QByteArray> send_buf;//发送缓冲区
////    json.insert("user",this->user_name);
////    json.insert("total",chunks.size()-1);
////    json.insert("filebody","");
////    json.insert("no",-1);
//    label_status->setText(tr("正在加密文件……"));
//    process_events_without_useript;
//    for(int i=0;i<chunks.size();i++){//生成发送内容
////        json["filebody"] = chunks[i];
////        json["no"] = i;
////        send_buf.push_back(encode(QJsonDocument(json).toJson()));
////        if(i%3000==0){
////            label_status->setText("正在加密文件…… 加密时间较长，请耐心等待");
////        }
//        header_filebody header;
//        header.check_type = mt_filebody;
//        header.total = chunks.size()-1;
//        header.no = i;
//        QByteArray msg;msg.append(reinterpret_cast<const char *>(&header),sizeof(header));msg.append(chunks[i]);
//        send_buf.push_back(encode(msg));
//        if(i%3==0)label_status->setText(QString(tr("正在加密文件…… %1/%2")).arg(i).arg(chunks.size()));
//        if(i%3==0)process_events_without_useript;
//    }
    
    
//    //分块发送
////    int loop_count = 0;//循环次数
//    / *const int DELAY_LOOP = 1;
//    for(int i=0;i<chunks.size();i++){
//        json["filebody"] = chunks[i];
//        json["no"] = i;
//        QEventLoop loop;
//        if(i % DELAY_LOOP == 0)QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
//        send(QJsonDocument(json).toJson());
//        if(i % DELAY_LOOP == 0)loop.exec(QEventLoop::ExcludeUserInputEvents);//循环两次停止一次
//        else for(int j=0;j<100;j++);//挨时间
//    }//*/
//    /*
//    //发送文件
//    QElapsedTimer clock;
//    clock.start();
//#ifdef Q_OS_WIN
//    timeBeginPeriod(1);
//#endif
//    QElapsedTimer clock_total;
//    clock_total.start();
//    QString str_debug;//调试字符串
    
//    const int DELAY_LOOP = 7;
//    const int PROCESS_LOOP = 1;
//    const int SEND_MAX_REQACKLOOP = 120;
//    const int SEND_MIN_REQACKLOOP = 5;
//    int display_lost = 0;//显示的丢包率
////    double value_plus_lost = 0;//对低延迟状态的丢包阈值增加
//    bool fast_plus = false;//快速恢复
//    double average_loss = 0.1;//平均丢包率
//    double average_good_loss = 0.01;//平均良好丢包率
//    double average_bad_loss = 0.3;//平均拥塞丢包率
//    sendState state = ss_normal_better;//发送当前状态
//    bool is_exploring = false;
//    double alpha = 0.02;
//    int stable_delay = SEND_MIN_DELAY;
//    int start_reqAck = 0;
//    int min_rtt = INT_MAX;
//    int current_rtt = 0;
//    QSet<int> skip;
//    bool is_first_explore = true;
    
//    line_delay->clear();
//    line_speed->clear();
//    line_ackloop->clear();
//    axis_y->setRange(0,SEND_MAX_DELAY);
//    axis_y_r->setRange(0,1000);
//    ui->tabWidget->setCurrentIndex(1);//切换到发送速度页
//    ui->statusBar->setMinimumHeight(55);
//    str_debug = QString("%1%2%3%4%5%6%7%8%9\n").arg("Time",13).arg("State",10).arg("AvgLoss",13).arg("AvgGLoss",13).arg("AvgBLoss",13).arg("Delay",10).arg("AckLoop",10).arg("LostRate",10).arg("Cong",10);
    
//    for(int i=0;i<send_buf.size();i++){
////        QEventLoop loop;
////        QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
//        if(!skip.contains(i))send(send_buf[i],0,currentSendDst);//不加密是因为已经加密过了
////        loop.exec(QEventLoop::ExcludeUserInputEvents);
        
//        if(i%7==0 || send_current_delay>50){
//            int pc=7;
//            if(send_current_delay>50)pc=1;
//            double speed = (pc * SPC/(clock.elapsed()/1000.0));
//            double dsp;//显示速度
//            QString dw="B";
//            if(speed>1024){
//                dw="KB";dsp=speed/1024;
//            }
//            if(speed>(1024*1024)){
//                dw="MB";dsp=speed/(1024*1024);
//            }
//            if(send_current_delay>20||i%4==0){
//                label_status->setText(QString(tr("发送文件中:包%1/%2 显示%3毫秒 速度%4%5/s \n本轮丢包%6% 延迟%7ms 验证轮%8个 累计%9 稳定%10\nGAB%12:%11:%13 状态%14 探索%15")).arg(i).arg(send_buf.size()-1).arg(clock.elapsed()).arg(dsp,0,'f',4).arg(dw).arg(display_lost).arg(send_current_delay).arg(send_req_ack_loop).arg(send_lost_loop_count).arg(stable_delay).arg(average_loss,0,'f',3).arg(average_good_loss,0,'f',3).arg(average_bad_loss,0,'f',3).arg(state).arg(is_exploring));
//                line_delay->append(i,send_current_delay);
//                line_ackloop->append(i,send_req_ack_loop);
//                line_speed->append(i,speed/1024.);//KB/s
//                axis_x->setRange(0,i+5);
//                if(speed > axis_y_r->max()) axis_y_r->setRange(0,speed+15);
//                chart_send->update();
//                ui->widget_info_chart1->update();
//                ui->label_info_time->setText(QString("共记录%1s").arg(clock_total.elapsed()/1000.));
//            }
//            clock.restart();
//        }
        
//        if(i%PROCESS_LOOP==0){
//            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents,1);
//        }
////        if(elapsed_lase_ack.elapsed() > send_current_delay + 80){
////            send_current_delay += 30;//强力降速
////            if(send_current_delay > SEND_MAX_DELAY){
////                send_current_delay = SEND_MAX_DELAY;
////            }
////            ndb<<"强力降速"<<send_current_delay;
////        }
        
        
////        if(i%2==0)send_current_delay+=1;
//        if(!skip.contains(i)){
//            if((i&DELAY_LOOP)==0)/*QThread::msleep(send_current_delay);* /multiDelay(send_current_delay);
//            else if(i%send_req_ack_loop == 0)/*QThread::msleep(2);* /multiDelay(2);//为了防止拥塞
//        }

        
//        //进行ACK请求
//        if(i%send_req_ack_loop == 0){
//            send_lost_count.clear();
//            ninfo<<"于包"<<i;
//            send_req_ack_uuid = QUuid::createUuid();
//            bool flag = false;
//            QEventLoop loop;
//            QTimer timer;
//            connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
//            connect(this,&MainWindow::signal_reqAck_finished,&loop,&QEventLoop::quit);
//            connect(this,&MainWindow::signal_reqAck_finished,this,[&,this]{flag=true;(void)this;});
//            timer.start(650);
//            send_current_reqAck = i;
//            ninfo<<"请求接收统计：从"<<start_reqAck<<"到"<<i;
//            QElapsedTimer clock1;clock1.start();
//            for(int ii=0;ii<3;ii++)send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8(),1,currentSendDst);
////            send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8());//防止丢包
//            int overdrop_cnt = 0;//透支包计数
//            QElapsedTimer clock_overdrop;clock_overdrop.start();
//            if(send_lost_loop_count > 2 && send_current_delay < 20){//网络条件好的时候 开始透支
//                for(;;){
//                    if(flag==true)break;
//                    if(clock_overdrop.elapsed()>min_rtt)break;
//                    if(overdrop_cnt >= qMin(send_req_ack_loop/2,5)) break;
//                    overdrop_cnt++;
//                    send(send_buf[i+overdrop_cnt],0,currentSendDst);
//                    skip.insert(i+overdrop_cnt);
//                    multiDelay(send_current_delay);
//                }
//                ninfo<<"透支了从"<<i+1<<"开始的"<<overdrop_cnt<<"个包";
//            }
//            if(!flag)loop.exec(QEventLoop::ExcludeUserInputEvents);
//            if(clock.elapsed()>10)ndb<<"请求所用时间>10ms:"<<clock.elapsed()<<"ms";
//            current_rtt = clock.elapsed();
//            if(current_rtt < min_rtt)min_rtt = current_rtt;
//            loop.deleteLater();
//            //开始计算条件
//            bool retry_flag = true;
//            bool congestion = false;
//            int last_lost_pack_index = -1;
//            int fat_cnt = 0;;
//            for(auto it=send_current_fastresend_map.rbegin();it!=send_current_fastresend_map.rend();it++,fat_cnt++){//检测是否因为缓冲区膨胀导致的丢包
//                if(last_lost_pack_index==-1)last_lost_pack_index=*it;
//                else if(abs(last_lost_pack_index-*it)>2){ 
//                    if(fat_cnt<3)retry_flag=false;
//                    break;
//                }
//            }
//            if(send_current_fastresend_map.size()<3)retry_flag=false;
//            QSet<int> old_fastresend = send_current_fastresend_map;
//            if(retry_flag && !send_current_fastresend_map.empty()){//重试
//                multiDelay(2);
//                send_lost_count.clear();
//                send_current_fastresend_map.clear();
//                ninfo<<"于包"<<i<<"开始重新请求统计信息";
//                send_req_ack_uuid = QUuid::createUuid();
//                bool flag = false;
//                QEventLoop loop;
//                QTimer timer;
//                connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
//                connect(this,&MainWindow::signal_reqAck_finished,&loop,&QEventLoop::quit);
//                connect(this,&MainWindow::signal_reqAck_finished,this,[&,this]{flag=true;(void)this;});
//                timer.start(600);
//                send_current_reqAck = i;
//                send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8(),1,currentSendDst);
//                loop.exec(QEventLoop::ExcludeUserInputEvents);
//                loop.deleteLater();
//                if(old_fastresend.size() - send_current_fastresend_map.size() < 2)congestion=true;//如果丢包集中在尾部且重新请求ACK无用就认为拥塞
//            }
//            float lost = 0;//平均丢包率
//            for(int i : send_lost_count){
//                lost += i;
//            }
//            ninfo<<"丢包数量"<<lost;
//            lost /= send_lost_count.size();
//            lost = lost / send_req_ack_loop;
//            ninfo<<"丢包率"<<lost;
//            ninfo<<"拥塞状态"<<congestion;
//            display_lost = lost * 100;//百分数
//            if(flag && lost <= 1.001){
//                if(1)if(lost!=0)ndb<<"此轮有丢包";
//                /*if(lost > 0.02){//丢包率大于2%
//                    if(send_lost_loop_count>0)send_lost_loop_count = 0;//清除连续不丢包记录
//                    send_lost_loop_count --;
//                    if(send_lost_loop_count < -3  ||  lost > 0.15){
//                        send_req_ack_loop -= 5;//如果多了就少发5个包
//                        if(send_req_ack_loop < 3)send_req_ack_loop=3;//不能太小
//                    }
//                    send_current_delay ++;
//                    if(lost > 0.3) send_current_delay += 10;//丢包太多快速减速
//                    if(send_current_delay > SEND_MAX_DELAY) send_current_delay=SEND_MAX_DELAY;
//                }
//                else{//丢包率少，提速
//                    int old_delay=send_current_delay;
//                    if(send_lost_loop_count<0)send_lost_loop_count = 0;//清除连续丢包记录
//                    send_lost_loop_count ++;
//                    if(send_lost_loop_count > 3 || lost <= 0.02){
//                        send_req_ack_loop += 5;
//                        if(send_req_ack_loop > 60)send_req_ack_loop=60;//不能大
//                    }
//                    send_current_delay -= 2;
//                    if(lost < 0.01) send_current_delay -= 10;//丢包太少快速加速
//                    if(lost == 0) send_current_delay -= 50;//0丢包飞快加速
//                    if(send_current_delay < SEND_MIN_DELAY) send_current_delay=SEND_MIN_DELAY;
//                    if(abs(send_current_delay-old_delay)>20 && send_req_ack_loop>=15) send_req_ack_loop=15;//快速提速需要增加请求频率
//                }* /
//                //快重传
//                if(1)/*调试禁用重传* /if(!send_current_fastresend_map.empty()){
//                    ninfo<<"快速重传"<<send_current_fastresend_map;
//                    foreach(int i , send_current_fastresend_map){
//                        send(send_buf[i],0,currentSendDst);
//                        multiDelay(send_current_delay);
//                        label_status->setText(QString("正在快速重传%1包").arg(i));
//                        if(i%2==0)process_events_without_useript;
//                    }
//                    multiDelay(1);
//                    send_current_fastresend_map.clear();
//                }
//                /*if(lost > 0.05+value_plus_lost){//丢包进行减速
//                    ndb<<"传输速率控制:丢包减速";
//                    if(send_lost_loop_count>0)send_lost_loop_count=0;
//                    else send_lost_loop_count--;//连续丢包
//                    if(lost < 0.1 && send_lost_loop_count==0){//首次丢包小幅度减速，不减小窗口
//                        send_current_delay ++;
//                        ndb<<"传输速率控制:+1,0\t\t[BAD]";
//                    }
//                    else if(lost < 0.15+value_plus_lost){//多次丢包遇到小丢包
//                        if( send_lost_loop_count > 3){//连续丢包较少，无惩罚延迟，减窗
////                            send_current_delay = send_current_delay / (1-lost);
//                            send_current_delay += lost*50;
//                            send_req_ack_loop -= 3;
//                            ndb<<"传输速率控制:+lost*50,-5\t\t[BAD]";
//                        }
//                        if(send_lost_loop_count <= 4){//多次丢包，剧烈减窗，判定为持续拥塞，进行智能控制
//                            send_current_delay = send_current_delay / (1-lost+0.05);
//                            send_req_ack_loop -= 8;
//                            ndb<<"传输速率控制:/(1-lost),-10\t\t[BAD]";
//                        }
//                    }
//                    else if(lost >= 0.15+value_plus_lost && (lost <= 0.25+value_plus_lost || send_req_ack_loop <30)){//多次丢包遇到中丢包，增加延迟，如果多次则增加窗口
////                        send_current_delay = send_current_delay / (1-lost) + 5;
//                        send_current_delay += lost*100;
//                        ndb<<"传输速率控制:+lost*100,0\t\t[BAD]";
//                        if(send_lost_loop_count<=3){//多次就增加延迟减窗
//                            send_req_ack_loop -= 10;
////                            send_current_delay += 5;
//                            send_current_delay = send_current_delay / (1-lost+0.5)-10;
//                            fast_plus=true;
//                            if(value_plus_lost>0)value_plus_lost=0;
//                            ndb<<"传输速率控制:/(1-lost)-10,-10\t\t[BAD]快速恢复正在排队";
//                        }
//                    }
//                    else if(lost > 0.25+value_plus_lost && send_req_ack_loop > 30){//大丢包剧烈减速
//                        send_current_delay = send_current_delay / (1-lost) + 15;
//                        send_current_delay = SEND_MAX_DELAY - 10;
//                        if(value_plus_lost>0)value_plus_lost=0;
//                        if(send_lost_loop_count<=3){//多次就重启
//                            send_req_ack_loop = SEND_MIN_REQACKLOOP + 2;
//                            send_current_delay = SEND_MAX_DELAY - 100;
//                            value_plus_lost=0;
//                            ndb<<"传输速率控制:[Restart]\t\t[BAD]";
//                        }
//                    }
//                    //规则检查
//                    int current_max_reqackloop = (send_current_delay>30?2000.:3500.)/send_current_delay;//要求至少2.5s更新一次
//                    if(send_req_ack_loop > current_max_reqackloop){
//                        send_req_ack_loop=current_max_reqackloop;
//                        ndb<<"传输速率控制:限制reqackloop"<<send_req_ack_loop;
//                    }
//                    //防止溢出
//                    send_current_delay = qBound(SEND_MIN_DELAY,send_current_delay,SEND_MAX_DELAY);
//                    send_req_ack_loop = qBound(SEND_MIN_REQACKLOOP,send_req_ack_loop,SEND_MAX_REQACKLOOP);
//                }
//                else{//丢包少
//                    ndb<<"传输速率控制:加速";
//                    if(send_lost_loop_count<0)send_lost_loop_count=0;
//                    else send_lost_loop_count++;//连续良好
//                    if(send_lost_loop_count == 0){//首次良好不过快加速
//                        send_current_delay -= 2;
//                        send_req_ack_loop += 2;
//                        ndb<<"传输速率控制:p1 -1.+2\t\t[GOOD]";
//                    }
//                    else if(send_lost_loop_count < 2){//3轮好，乘法加速+增窗1
//                        send_current_delay *= 0.85;
//                        send_req_ack_loop += 5;
//                        ndb<<"传输速率控制:p2 *0.9,+2\t\t[GOOD]";
//                    }
//                    else if(send_lost_loop_count >= 2 && send_lost_loop_count < 6){//更快乘法加速，普通增窗
//                        send_current_delay *= 0.8;
//                        send_req_ack_loop += 5;
//                        ndb<<"传输速率控制:p3 *0.8,+2\t\t[GOOD]";
//                    }
//                    else if(send_lost_loop_count >= 6){//5轮好，1剧烈乘法加速但不增窗 2线性加速+增窗
//                        if(send_current_delay > SEND_MIN_DELAY + 35){//方案1
//                            send_current_delay *= 0.7;
//                            send_req_ack_loop += 5;
//                            ndb<<"传输速率控制:p4a *0.75,+1\t\t[GOOD]";
//                        }
//                        else{//方案2
//                            send_current_delay -= 8;
//                            send_req_ack_loop += 15;
//                            value_plus_lost = 0.08;//宽容8%丢包
//                            ndb<<"传输速率控制:p4b -5,+10\t\t[GOOD]";
//                        }
//                    }
//                    //快速恢复
//                    if(fast_plus){
//                        ndb<<"传输速率控制：快速恢复";
//                        if(send_current_delay > 20){
//                            send_current_delay *= 0.5;
//                            send_req_ack_loop += 1;
//                        }
//                        else{
//                            send_current_delay -= 5;
//                        }
//                        fast_plus=false;
//                    }
//                    //防止溢出
//                    send_current_delay = qBound(SEND_MIN_DELAY,send_current_delay,SEND_MAX_DELAY);
//                    send_req_ack_loop = qBound(SEND_MIN_REQACKLOOP,send_req_ack_loop,SEND_MAX_REQACKLOOP);
//                }* /
                
//                //拥塞控制算法
//                //状态更新
//                if(lost <= average_loss){
//                    if(lost <= average_good_loss) state=ss_excellent;
//                    else{
//                        if(abs(lost-average_good_loss) <= abs(lost-average_loss)) state=ss_good;
//                        else state=ss_normal_better;
//                    }
//                }
//                else{
//                    if(lost > average_bad_loss*1.05) state=ss_worst;
//                    else{
//                        if(abs(lost-average_bad_loss) < abs(lost-average_loss)) state=ss_bad;
//                        else state=ss_normal_worse;
//                    }
//                }
//                //计算平均
//                average_loss = alpha * lost + (1-alpha) * average_loss;
//                switch (state) {
//                case ss_excellent:
//                case ss_good:
//                    average_good_loss = alpha * lost + (1-alpha) * average_good_loss;
//                    break;
//                case ss_worst:
//                case ss_bad:
//                    average_bad_loss = alpha * lost + (1-alpha) * average_bad_loss;
//                    break;
//                default:
//                    break;
//                }
//                //RTT控制
////                if(current_rtt > min_rtt + 35){
//////                    congestion = true;
////                    if(stable_delay < send_current_delay - 5){
////                        stable_delay = send_current_delay - 4;
////                    }
////                    min_rtt ++;
////                }
//                //根据状态控制加减速
//                switch (state) {
//                case ss_excellent://良好状态进行加速
//                    if(send_lost_loop_count<0) send_lost_loop_count=0;
//                    else send_lost_loop_count++;
//                    //进行加减速
//                    if(is_exploring){//探索中 线性加速
//                        send_current_delay -= 5;
//                        send_req_ack_loop -= 0;
//                    }
//                    else{
//                        if(!congestion){
//                            send_current_delay *= 0.8;
//                            send_req_ack_loop *= 1.2;
//                        }
//                        else{
////                            multiDelay(1);
//                            average_loss -= 0.005;//既然拥塞了都能进ss_excellent就说明average_loss被污染
//                            average_good_loss -= 0.005;
//                        }
//                    }
//                    //探索控制
//                    if(send_current_delay < stable_delay && send_current_delay>SEND_MIN_DELAY){//需要探索
//                        if(send_lost_loop_count > 2){//连续稳定允许探索
//                            is_exploring = true;
//                            if(send_req_ack_loop > 50){
//                                send_req_ack_loop = 50;//不允许过大
//                            }
//                        }
//                        else{
//                            send_current_delay = stable_delay;
//                        }
//                    }
//                    if(is_exploring && send_lost_loop_count > 5){
//                        stable_delay = send_current_delay + 1;
//                        is_exploring = false;
//                        send_lost_loop_count = 0;
//                    }
//                    break;
//                case ss_good://较好
//                    if(send_lost_loop_count<0) send_lost_loop_count=0;
//                    else send_lost_loop_count++;
//                    //进行加减速
//                    if(is_exploring){//探索中 线性加速
//                        send_current_delay -= 1;
//                    }
//                    else{
//                        if(!congestion){
//                            send_current_delay -= 5;
//                            send_req_ack_loop += 5;
//                        }
//                        else{
                            
//                        }
//                    }
//                    //探索控制
//                    if(send_current_delay < stable_delay && send_current_delay>SEND_MIN_DELAY){//需要探索
//                        if(send_lost_loop_count>4){
//                            is_exploring = true;
//                            if(send_req_ack_loop > 50){
//                                send_req_ack_loop = 50;//不允许过大
//                            }
//                        }
//                        else{
//                            send_current_delay = stable_delay;
//                        }
//                    }
//                    if(is_exploring && send_lost_loop_count > 4){
//                        stable_delay = send_current_delay + 5;
//                        is_exploring = false;
//                        send_lost_loop_count = 0;
//                    }
//                    break;
//                case ss_normal_better:
////                    if(send_lost_loop_count<0) send_lost_loop_count=0;
////                    else send_lost_loop_count++;
//                    //进行加减速
//                    if(is_exploring){//停止探索
//                        is_exploring = false;
//                        stable_delay = send_current_delay + 1;
//                        send_lost_loop_count = 0;
//                    }
//                    else{
////                        if(send_lost_loop_count<0)send_current_delay -= 1;
//                        if(congestion){
//                            multiDelay(1);
//                        }
//                    }
//                    //探索控制
//                    if(send_current_delay < stable_delay){//不允许探索
//                    }
//                    break;
//                case ss_normal_worse:
//                    if(send_lost_loop_count>0) send_lost_loop_count=0;
//                    else send_lost_loop_count--;
//                    //进行加减速
//                    if(is_exploring){//停止探索，单轮延迟
//                        is_exploring = false;
//                        if(send_current_delay+5<stable_delay)stable_delay=send_current_delay+5;
//                        multiDelay(5);
//                    }
//                    else{
//                        send_current_delay += 1;
////                        if(send_lost_loop_count < -5){
////                            if(stable_delay > send_current_delay){//更新stable_delay
////                                stable_delay = send_current_delay - 5;
////                                send_lost_loop_count = 0;
////                            }
////                        }
//                        if(congestion){
//                            multiDelay(3);
//                        }
//                    }
//                    if(is_first_explore){
//                        is_first_explore=false;
//                        stable_delay=send_current_delay;
//                    }
//                    break;
//                case ss_bad:
//                    if(send_lost_loop_count>0) send_lost_loop_count=0;
//                    else send_lost_loop_count--;
//                    //进行加减速
//                    if(is_exploring){//停止探索，单轮延迟，回归初始延迟
//                        is_exploring = false;
//                        multiDelay(5);
//                        send_current_delay = stable_delay;
//                    }
//                    else{
//                        send_current_delay += 3;
//                        send_req_ack_loop -= 5;
//                        if(send_lost_loop_count < -2){
//                            if(stable_delay > send_current_delay){//更新stable_delay
//                                stable_delay = send_current_delay - 5;
//                            }
//                        }
//                        if(congestion){
//                            send_current_delay+=1;
//                            multiDelay(5);
//                        }
//                    }
//                    if(is_first_explore){
//                        is_first_explore=false;
//                        stable_delay=send_current_delay+3;
//                    }
//                    break;
//                case ss_worst:
//                    if(send_lost_loop_count>0) send_lost_loop_count=0;
//                    else send_lost_loop_count--;
//                    //进行加减速
//                    if(is_exploring){//停止探索，单轮延迟，回归初始延迟-8
//                        is_exploring = false;
//                        multiDelay(8);
//                        send_current_delay = stable_delay-8;
//                    }
//                    else{
////                        int old_send_current_delay=send_current_delay;
////                        send_current_delay *= 1.2;
////                        if(abs(old_send_current_delay-send_current_delay)<10)send_current_delay+=10;
//                        send_current_delay += 10;
//                        send_req_ack_loop -= 10;
//                        if(send_lost_loop_count < -2){
//                            if(stable_delay > send_current_delay){//更新stable_delay
//                                stable_delay = send_current_delay - 10;
//                            }   
//                            send_current_delay += 15;
//                        }
//                        if(congestion)multiDelay(5);
//                    }
//                    if(is_first_explore){
//                        is_first_explore=false;
//                        stable_delay=send_current_delay+8;
//                    }
//                    break;
//                }
//                //限制范围
//                send_current_delay = qBound(SEND_MIN_DELAY,send_current_delay,SEND_MAX_DELAY);
//                send_req_ack_loop = qBound(SEND_MIN_REQACKLOOP,send_req_ack_loop,SEND_MAX_REQACKLOOP);
//                //输出信息
//                ninfo<<"控制统计信息："<<QString("延迟=%1,验证轮=%2,状态机状态=%3,平均丢包=%4,正常网络状态下的平均丢包=%5,拥塞状态下的平均丢包=%6,连续状态计数=%7,探索状态=%8,拥塞=%9").arg(send_current_delay).arg(send_req_ack_loop).arg(state).arg(average_loss).arg(average_good_loss).arg(average_bad_loss).arg(send_lost_loop_count).arg(is_exploring?"true":"false").arg(congestion?"true":"false");
//            }
//            else{
//                send_current_delay += 3;//如果连控制包都丢了，就减速
//                if(send_lost_loop_count>0)send_lost_loop_count=0;
//                else send_lost_loop_count --;
//                average_loss += 0.03;
//                multiDelay(10);
//                send_req_ack_loop -= 5;
//                if(send_req_ack_loop < 3)send_req_ack_loop=3;//不能太小
//                if(send_current_delay > SEND_MAX_DELAY) send_current_delay=SEND_MAX_DELAY;
//                fast_plus = true;//控制包丢失启用快速恢复
//                int current_max_reqackloop = (send_current_delay>30?2000.:3500.)/send_current_delay;//要求至少2.5s更新一次
//                if(send_req_ack_loop > current_max_reqackloop){
//                    send_req_ack_loop=current_max_reqackloop;
//                    ninfo<<"传输速率控制:限制reqackloop"<<send_req_ack_loop;
//                }
//                ninfo<<"控制包丢失";
//            }
//            ninfo<<"var:send_current_delay ="<<send_current_delay;
////            str_debug.append(QString("%4:\t%1 \t\t%2 \t%3\n").arg(lost).arg(send_current_delay).arg(send_req_ack_loop).arg(i));
//            str_debug.append(QString("%1%2%3%4%5%6%7%8%9\n").arg(QTime::currentTime().toString("hhmmsszzz"),13).arg(state,10).arg(average_loss,13,'f',4).arg(average_good_loss,13,'f',4).arg(average_bad_loss,13,'f',4).arg(send_current_delay,10).arg(send_req_ack_loop,10).arg(lost,10).arg(congestion?"True":"False",10));
//            if(flag)start_reqAck = i+1;//在这里更新因为为了预防控制包丢失导致无法快速重传和漏轮的情况
////            i += overdrop_cnt - 1;//已经透支过的包不再发送
//        }
//    }
    
//    label_status->setText(tr("发送文件完毕"));
//    if(1)ui->textEdit_debug1->setText(str_debug);
//    ui->statusBar->setMinimumHeight(0);
//    timer_is_uploading.start(10000);
//    int spd = fileList.size() / (clock_total.elapsed()/1000.);
//    ui->label_info_time->setText(QString("已记录%1s(%5),%2(%3,%4)").arg(clock_total.elapsed()/1000.,0,'f',2).arg(spd).arg(spd/1024.).arg(spd/1024./1024).arg(fileList.size()));
//    ui->textBrowser_debug1->append(str_debug);
//#ifdef Q_OS_WIN
//    timeEndPeriod(1);
//#endif
//}
//*/

//void MainWindow::sendFile(QList<device> dst){
//    if(dst.empty()){//让用户选择
//        Dialog_selectSyncDst *dialog = new Dialog_selectSyncDst(this);
//        auto tmp = clients;tmp.removeAll(public_ip);
//        dialog->setup(tmp);
//        connect(dialog,&Dialog_selectSyncDst::syncdstDecided,this,[&](QList<device> a){dst=a;});
//        dialog->exec();
//        dialog->deleteLater();
//        ninfo<<"SyncDst:"<<dst;
//    }
//    if(dst.empty()){
//        QMessageBox::warning(this,tr("同步文件"),tr("请选择目标！"));
//        return;
//    }
//    dst.removeAll(public_ip);//文件不发给自己
//    lastSyncDst=dst;
////    //开始规划
////    auto plan = planAutoSend(dst);
////    auto senders = dst;
////    senders.append(public_ip);
////    //提取每个人任务
////    QList<QString> tasks;//索引和senders一一对应
////    for(int i=0;i<senders.size();i++){
////        QStringList task;
////        foreach(auto j , plan){
////            foreach(auto k , j){
////                if(k.first==senders[i]){
////                    task.append(k.second);
////                }
////            }
////        }
////        tasks.append(task.join(';'));
////    }
////    //通知任务
////    QStringList self_tasks;
////    for(int i=0;i<senders.size();i++){
////        if(!(senders[i]==public_ip)){
////            if(!tasks[i].isEmpty()){
////                sendReliableMessage(clients.indexOf(senders[i]),"SEND_TASK"+tasks[i]);
////            }
////        }
////        else self_tasks = tasks[i].split(';');
////    }
    
////    //发送
////    for(auto t : self_tasks){
////        //查找
////        int index = -1;
////        for(int i=0;i<clients.size();i++){
////            if(clients[i].operator QString const() == t){
////                index=i;
////                break;
////            }
////        }
//////        QMetaObject::invokeMethod(this,[=]{sendFileTo(index);},Qt::QueuedConnection);//QueuedConnection在事件循环运行并且顺序按照invoke的顺序运行。sendFileTo不能在除了事件循环以外的其他地方运行
////        sendFileTo(index);
////        ui->textEdit_debug1->append(QString("发送文件到%1").arg(index));
////    }
//    m_transmissionengine->SPTP_send(mergeFile(QDir("files/")),dst);
//}


//void MainWindow::releaseFile(QByteArray msg){   
////    QTextStream stm(&msg);
////    QStack<QDir> stack;
////    stack.push(QDir("files/"));
    
//    while(!msg.isEmpty()){//循环读取
////        QString operation = stm.readLine();
//        QString operation = readLine(msg);
        
//        //解析
//        if(operation == "FILE"){//解析文件
//            QString filename = /*stm.readLine()*/readLine(msg);//获取文件名
//            bool enableAttackCheck = true;
//            if(QFile(filename+".private.stlink").exists()){
//                QFile f(filename+".private.stlink");f.open(QFile::ReadOnly);
//                filename=QDir(f.readAll()).absolutePath();
//                enableAttackCheck=false;
//            }
//            QFile file(/*stack.top().filePath(filename)*/filename);
//            file.open(QIODevice::WriteOnly);
//            if(!file.isOpen()){
//                ninfo<<"Error:File isn't open.Details:"<<file.errorString();
//            }
//            int size = /*stm.readLine()*/readLine(msg).toInt();
//            QByteArray value = qUncompress(msg.left(size));
//            msg = msg.mid(size);
////            QByteArray value = qUncompress(QByteArray::fromBase64(stm.readLine().toUtf8()));//文件内容
            
//            //攻击检测
//            if((enableAttackCheck && !filename.startsWith("files/")) || filename.contains("..")){//路径便利攻击
//                bool f = filename.contains(".dll")||filename.contains(".exe")||filename.contains(".nprivate")||filename.contains("config.json")||filename.contains(".ini")||filename.contains(".sys");
//                bool isSystemPath = filename.contains("/Windows/") || 
//                                     filename.contains("/System/") ||
//                                     filename.contains("/etc/") ||
//                                     filename.contains("/bin/");
//                attackProtection(f?at_minor:at_influential,
//                                 QString(tr("识别到文件%1不以files/开头，试图%2，有极大可能是路径遍历攻击\n影响：可能修改应用程序的配置文件或依赖文件，导致木马病毒、勒索程序或其他恶意程序被“正当”地安装并打开")).arg(filename).arg(isSystemPath?"篡改系统关键文件":"越权访问危险目录"),
//                                 QString(tr("停止释放文件'%1'(恢复方法:重新发送不含攻击内容的文件)")).arg(filename),
//                                 [&]{file.close();});
//            }
//            file.write(value);
//            file.close();
//            ninfo<<"释放文件"<<filename<<"大小"<<value.size();
////            stm.readLine();
//        }
        
//        if(operation == "DIR"){//处理目录
//            /*QString dirName = stm.readLine();
            
//            QDir currentDir = stack.top();
//            if(!QDir(currentDir.filePath(dirName)).exists()){
//                currentDir.mkpath(dirName);
//            }
            
////            stack.push(QDir(QDir(stack.top()).filePath(dirName)));*/
            
////            QDir dir = stm.readLine();
//            QDir dir = QString(readLine(msg));
//            dir.mkpath(".");
//            ninfo<<"创建目录"<<dir;
//        }
        
////        if(operation == "END_DIR"){//处理目录
////            if(!stack.empty()){
////                stack.pop();
////            }
////        }
//    }
    
//    //刷新目录
//    show_dir();
//    label_status->setText(tr("释放文件成功"));
//    trayIcon->showMessage(windowTitle(),tr("文件释放成功！"),QSystemTrayIcon::Information);
//    //刷新记录避免错误同步
//    fileHashMap=generateFileHashMap(syncFolder);
//}


//void MainWindow::savePower(){
//    if(!widget_savePower) widget_savePower = new QWidget(this);
//    widget_savePower->installEventFilter(this);
    
//    //创建黑色窗口
//    widget_savePower->setObjectName("savePW");
//    widget_savePower->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
//    widget_savePower->setStyleSheet("#savePW { background-color: black !important; }\nQLabel{ color: white; }\nQMainWindow{ background-color: black !important; }");
//    widget_savePower->setGeometry(QApplication::primaryScreen()->geometry());
    
//    //添加label
//    auto layout = (new QVBoxLayout(this));layout->addWidget(label_status);
//    widget_savePower->setLayout(layout);
    
//    widget_savePower->show();
//    timer_savePower.start(1000);
//}


//bool MainWindow::checkSkin(MainWindow::skinType skin){
//    if(skin<Silver) return true;
    
//    //查找请求地址
//    QUrl req_url;
//    switch (skin) {
//    case Golden:
//        req_url = QUrl("https://cfpages.nnpyro.fwh.is/synctunnel-interface/skin-golden-1.0.txt");
//        break;
//    case Silver:
//        req_url = QUrl("https://cfpages.nnpyro.fwh.is/synctunnel-interface/skin-silver-1.0.txt");
//        break;
//    default:
//        ninfo<<"ERROR Unexcept skin"<<skin;
//    }
    
    
//    //请求
//    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
//    QNetworkRequest request(req_url);
//    QNetworkReply *reply = manager->get(request);
    
//    //等待
//    QEventLoop loop;
//    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
//    QTimer::singleShot(6000,&loop,&QEventLoop::quit);
//    loop.exec();
    
//    //处理
//    if(reply->error() != QNetworkReply::NoError){
//        QMessageBox::critical(this,tr("样式查询"),tr("连接至服务器错误，无法使用限定款UI界面。\nError:")+reply->errorString());
//        delete manager;
//        return false;
//    }
//    QString msg = reply->readAll();
//    QTextStream stm(&msg);
//    QString machineID = QSysInfo::machineUniqueId();
//    for(;!stm.atEnd();){
//        QString id = stm.readLine();
//        if(id==machineID){
//            delete manager;
//            return true;
//        }
//    }
//    return false;
//}


//void MainWindow::copy_remote_file(int index){
//    dialog_remoteFile = new Dialog_remoteFile(this);
//    QString drive = QInputDialog::getText(this,tr("远程复制文件"),tr("请输入您需要查看的盘符，例如C,D,E等，只包括字母部分"),QLineEdit::Normal,"C") + ":/";
    
//    connect(dialog_remoteFile,&Dialog_remoteFile::request_fileList,this,[=,this](QString dir){
//        send(QString("{\n    \"get_folderList\":\"%1\"\n}").arg(dir).toUtf8(),1,index);
//    });
//    connect(dialog_remoteFile,&Dialog_remoteFile::ok,this,[=,this](QString f){
//        send(QString("{\n    \"copy\":\"%1\"\n}").arg(f).toUtf8(),1,index);
//    });
    
//    send(QString("{\n    \"get_folderList\":\"%1\"\n}").arg(drive).toUtf8(),1,index);
//    dialog_remoteFile->exec();
//    delete dialog_remoteFile;
//}


//void MainWindow::multiDelay(float ms)
//{
//#ifdef Q_OS_WIN
//    LARGE_INTEGER start, end, freq;
//    QueryPerformanceFrequency(&freq);  // 获取计数器频率
//    QueryPerformanceCounter(&start);   // 获取开始时间
    
//    // 计算目标计数值
//    LONGLONG targetCount = start.QuadPart + 
//        static_cast<LONGLONG>(((ms-0.5) / 1000.0) * freq.QuadPart);
    
//    // 智能忙等待循环
//    while (true) {
//        QueryPerformanceCounter(&end);
//        if (end.QuadPart >= targetCount) break;  // 达到目标时间
        
//        // CPU优化策略
//        LONGLONG remainingCount = targetCount - end.QuadPart;
//        float remainingMs = (remainingCount * 1000.0f) / freq.QuadPart;
        
//        if(remainingMs>20.){
//            Sleep(0);
//        }
//    }
//#else
//    // 其他平台的实现
//    QThread::usleep(static_cast<useconds_t>(ms * 1000));
//#endif
//}


//QByteArray MainWindow::readLine(QByteArray &ba){
//    QByteArray ret;
//    int newLinePos = ba.indexOf("\n");
//    if(newLinePos == -1){
//        ret = ba;
//        ba.clear();
//        return ret;
//    }
//    else{
//        ret = ba.left(newLinePos+1);
//        if(ret.endsWith("\r\n"))ret.chop(2);
//        if(ret.endsWith("\n"))ret.chop(1);
//        ba = ba.mid(newLinePos+1);
//    }
//    return ret;
//}


//void MainWindow::attackProtection(MainWindow::attack_type type, QString reason, QString solution, MainWindow::fpvoid solution_fp, MainWindow::fpvoid rec){
//    solution_fp();//先解决
//    trayIcon->showMessage(tr("SyncTunnel 网络攻击防护"),QString(tr("您的设备正在遭受%1级别（最高5级）的网络攻击，已自动为您处理。\n打开SyncTunnel显示详细信息。")).arg(type+1),QSystemTrayIcon::Warning);
//    QMessageBox::warning(this,"网络攻击防护",QString(tr("您的设备似乎正在遭受%1级别（最高5级）的网络攻击。已经自动处理。\n\n网络攻击详细信息：\n%2\n\n已经采取的处理办法：\n%3")).arg(type).arg(reason).arg(solution));
//    if(rec!=0){
//        bool needRec = QMessageBox::information(this,tr("网络攻击防护"),tr("这次的攻击防护提供了恢复方法，如果您认为这次攻击防护是误操作，可以恢复防护前的状态。\n\n您希望恢复吗？"),QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes;
//        if(needRec){
//            rec();//恢复
//        }
//    }
//}


MainWindow::MainWindow(ViewModel *vm, QWidget *parent, std::function<void (QString)> func_update, bool bShow):
    QMainWindow(parent),
    vm(vm),
    ui(new Ui::MainWindow)
{
    //对象创建
    label_status = new QLabel(this);
    logFile = new QFile(QString("logs/%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")).toStdString().c_str(),this);
    //基本设置
    ui->setupUi(this);
    resize(QGuiApplication::primaryScreen()->geometry().width()*0.45,QGuiApplication::primaryScreen()->geometry().height()*0.55);
    setWindowTitle("SyncTunnel 同步隧道");
    ui->tabWidget->setCurrentIndex(0);
//    timer_is_uploading.setSingleShot(true);
//    timer_clear_currentFileMap.setSingleShot(true);
    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置自动列宽
    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);//设置自动列宽
    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);//设置自动列宽
    setAcceptDrops(true);//接收拖放
    ui->listWidget_file->setAcceptDrops(true);// 允许接收拖放事件
    ui->listWidget_file->setDragDropMode(QAbstractItemView::NoDragDrop);//让事件传递到MainWindow
#ifdef Q_OS_WIN
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);//设置优先级为高
#endif
    {//添加Item   必需和枚举的一一对应
        ui->comboBox_settings_uiskin->addItem(tr("科技蓝（默认）"),BusinessLogic::Dark);
        ui->comboBox_settings_uiskin->addItem(tr("纯洁白"),BusinessLogic::Light);
        ui->comboBox_settings_uiskin->addItem(tr("银色星辰（限定版，前50个下载本软件的用户可申请）"),BusinessLogic::Silver);
        ui->comboBox_settings_uiskin->addItem(tr("金色流光（限定版，对本软件的创作有突出贡献的用户可申请）"),BusinessLogic::Golden);
    }
//    ui->tableWidget_deviceList.
    setWindowIcon(QIcon(":/rc/img/favicon.ico"));
    trayIcon->setIcon(QIcon(":/rc/img/favicon.ico")); 
    trayIcon->setContextMenu(new QMenu(this));
    trayIcon->contextMenu()->addAction(ui->actionExit_Application);
    trayIcon->show();
    trayIcon->setVisible(true);
    // ui->label_remote_screen->installEventFilter(this);
//    line_ackloop->setColor(QColor(Qt::green));
//    line_speed->setColor(QColor(Qt::red));
//    line_delay->setColor(QColor(Qt::yellow));
//    line_ackloop->setName("检验轮个数");
//    line_delay->setName("发送延迟");
//    line_speed->setName("发送速度");
//    chart_send->addSeries(line_speed);
//    chart_send->addSeries(line_ackloop);
//    chart_send->addSeries(line_delay);
//    chart_send->addAxis(axis_x,Qt::AlignBottom);
//    chart_send->addAxis(axis_y,Qt::AlignLeft);
//    chart_send->addAxis(axis_y_r,Qt::AlignRight);
//    ui->widget_info_chart1->setChart(chart_send);
//    line_speed->attachAxis(axis_x);line_speed->attachAxis(axis_y_r);
//    line_ackloop->attachAxis(axis_x);line_ackloop->attachAxis(axis_y);
//    line_delay->attachAxis(axis_x);line_delay->attachAxis(axis_y);
    //隐藏不必要的标签页
//    vm->restartDebug();
    hideTab(ui->tabWidget,2);
    hideTab(ui->tabWidget,3);
    hideTab(ui->tabWidget,5);
    hideTab(ui->tabWidget,6);
    ui->tabWidget->tabBar()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //取消鼠标滚轮
    ui->spinBox_settings_mqttPort->installEventFilter(this);
    ui->comboBox_settings_uiskin->installEventFilter(this);
    ui->comboBox_settings_language->installEventFilter(this);
#ifdef NNPYRO_USE_CONSOLE//使用控制台
    if(!QApplication::arguments().contains("CON_MODE")){if(!QDir("tools/").exists())QDir().mkpath("tools/");QFile::copy(":/rc/bin/Alacritty.exe","tools/Alacritty.exe");QProcess::startDetached("tools/Alacritty.exe",QStringList()<<"-e"<<QApplication::applicationFilePath()<<QApplication::arguments()<<"CON_MODE");close();QApplication::quit();}
#endif
#ifdef NNPYRO_COLORFULCON//使用控制台
    qInstallMessageHandler(static_cast<QtMessageHandler>(log));
#endif
    ui->statusBar->addPermanentWidget(label_status);
//    vm->o_status.use(qApp,[=]{ndb<<vm->o_status;});
    ui->widget_remoteControl->setEnabled(false);
    
    //信号槽绑定
    connect(ui->listWidget_file,&QListWidget::doubleClicked,this,&MainWindow::on_folder_change);
    connect(ui->listWidget_file,&QListWidget::customContextMenuRequested,this,&MainWindow::on_rightclick);
    connect(ui->actionOpen,&QAction::triggered,this,[vm,this]{
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(vm->o_current_dir.get().filePath(ui->listWidget_file->currentItem()->text())).absolutePath()));
    });
    connect(ui->actionFolder,&QAction::triggered,this,[vm,this]{/*QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators((current_dir).absolutePath())));*/QDesktopServices::openUrl(QUrl::fromLocalFile(vm->o_current_dir.get().absolutePath()));});
    connect(ui->pushButton_settings_save,&QPushButton::clicked,this,&MainWindow::on_settings_saved);
    connect(ui->actionupload_file,&QAction::triggered,this,[this]{sendFile();});
//    connect(ui->actionHangup,&QAction::triggered,this,&MainWindow::on_hangup);
//    connect(ui->actionDownload,&QAction::triggered,this,&MainWindow::on_download);
//    connect(ui->actionSync_PAT,&QAction::triggered,this,[this]{
//        QJsonObject json;json.insert("pat",ui->lineEdit_settings_githubPAT->text());
//        send(QJsonDocument(json).toJson());
//    });
    connect(ui->checkBox_settings_ipv6,&QCheckBox::clicked,this,[this](bool isCheck){
        if(isCheck)QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments()<<("-ipv6"));
        else{
            QStringList l=QApplication::arguments();l.removeAll("-ipv6");
            QProcess::startDetached(QApplication::applicationFilePath(),l);
        }close();
    });
    connect(ui->actionRefresh,&QAction::triggered,this,[this]{show_dir();});
//    connect(process_proxy,&QProcess::readyRead,this,[this]{ui->textBrowser_proxy->append(QString::fromLocal8Bit(process_proxy->readAll()));});
//    connect(&timer_savePower,&QTimer::timeout,this,[this]{
//        label_status->setText(QString(tr("SyncTunnel正在等待文件传输请求……\n如果没有请求，计算机将会在%1秒后关闭\n您也可以用Alt+F4关闭此窗口后手动关机")).arg(/*QTime().addMSecs(timer_savePower_finish.remainingTime()).toString("HH时mm分ss秒"))*/timer_savePower_finish.remainingTime()/1000));
//    });
#ifdef Q_OS_WIN
//    connect(&timer_savePower_finish,&QTimer::timeout,this,[this]{
//        ShutdownBlockReasonDestroy((HWND)winId());
//        QProcess::startDetached("shutdown",{"-s","-t","10"});(void)this;
//    });
    connect(ui->actionShutdown,&QAction::triggered,this,[this]{ShutdownBlockReasonDestroy((HWND)winId());QProcess::startDetached("shutdown",{"-s","-t","10"});});
#endif
    connect(ui->actionShutdown_current,&QAction::triggered,this,[vm,this]{int i=ui->tableWidget_deviceList->currentRow();if(i<0){QMessageBox::warning(this,"请先选中一个设备","请先选中一个设备");return;}vm->on_shutdown_current(i);});
    connect(ui->tableWidget_deviceList,&QTableWidget::customContextMenuRequested,this,&MainWindow::on_rightclick_deviceList);  
    connect(ui->actionTest_RTT,&QAction::triggered,this,&MainWindow::on_test_rtt);
    connect(ui->pushButton_settings_requestUI,&QPushButton::clicked,this,[]{
        QUrl url("mailto:nnpyro2@outlook.com");
        QUrlQuery query;
        query.addQueryItem("subject","SyncTunnel申请限定款UI");
        query.addQueryItem("body",QString("您好！\n我是SyncTunnel的用户，我想要申请(填写您需要申请的界面样式 金色流光/银色星辰)UI样式。\n\n我对软件开发创作做出了如下突出贡献\n（请填写此处。或者您可以填写您是前50位下载本软件的用户）\n\n我的设备唯一ID：\n%1").arg(QString(QSysInfo::machineUniqueId())));
        url.setQuery(query);QDesktopServices::openUrl(url);
    });
    connect(trayIcon,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason reason){Q_UNUSED(reason);if(isMinimized())showNormal();if(isHidden())show();raise();});
    connect(ui->pushButton_copyId,&QPushButton::clicked,this,[this]{QApplication::clipboard()->setText(QSysInfo::machineUniqueId());QMessageBox::information(this,"SyncTunnel","复制成功！");});
    connect(ui->actionRequestFile,&QAction::triggered,this,[vm,this]{int i=ui->tableWidget_deviceList->currentRow();if(i==-1){QMessageBox::warning(this,"错误","请先选中一个设备");return;} vm->on_request_file(i);label_status->setText("正在等待发送方响应...");});
//    connect(&timer_keepAlive,&QTimer::timeout,this,[this]{if(chunks.empty()&&currentFileMap.empty())send("KEEP_ALIVE");});
//    connect(ui->actionRemoteCopyFile, &QAction::triggered, this, [this] {int currentRow = ui->tableWidget_deviceList->currentRow();if (currentRow == -1) {QMessageBox::warning(this, "错误","你需要先选中一个设备");return;}copy_remote_file(currentRow);});
//    connect(ui->actionSend_message,&QAction::triggered,this,[this]{int index=ui->tableWidget_deviceList->currentRow();if(index==-1){QMessageBox::warning(this,"发送测试消息","请先选中一个设备");return;} QString msg=QInputDialog::getText(this,"发送测试消息","请输入测试消息：");QJsonObject json;json.insert("test_msg",QString(msg.toUtf8()));send(QJsonDocument(json).toJson(),1,index);});
    connect(ui->actionOpen_DriveCrypto,&QAction::triggered,this,[]{QProcess::startDetached("DriveCrypto.exe");});
    connect(ui->pushButton_settings_getDefaultPAT,&QPushButton::clicked,this,[this]{
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request;
        request.setUrl(QUrl("https://nnpyro.netlify.app/synctunnel-interface/github_pat.txt"));
        request.setHeader(QNetworkRequest::UserAgentHeader,"nnpyro SyncTunnel vbeta-x.x");
        QNetworkReply *reply=manager->get(request);
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        if(reply->error() != QNetworkReply::NoError) QMessageBox::critical(this,"错误","错误"+reply->errorString());
        else {ui->lineEdit_settings_githubPAT->setText(reply->readAll());ui->lineEdit_settings_gitubUser->setText("nnpyro1");}
    });
    connect(shortcut_debug,&QShortcut::activated,this,[this]{is_debug=is_debug?false:true;restartDebug();QMessageBox::information(this,"debug",QString("您已%1调试模式").arg(is_debug?"进入":"离开"));});
    connect(ui->pushButton_settings_recordLog,&QPushButton::pressed,this,[this]{if(!QDir("logs").exists())QDir("logs").mkpath(".");logFile->open(QFile::WriteOnly);ninfo<<"类"<<this<<"将日志输出重定向到文件";output_to_file = true;qInstallMessageHandler(static_cast<QtMessageHandler>(log));ninfo<<"日志输出重定向成功！";});
    connect(ui->actionAdd_file,&QAction::triggered,this,[vm,this]{QString dir=QFileDialog::getOpenFileName(this);QFile(dir).copy(vm->o_current_dir.get().absolutePath());});
    connect(ui->actionIncremental_sync_enable,&QAction::triggered,this,[vm,this]{
        vm->incremental_sync_set.insert(QDir("files").relativeFilePath(vm->o_current_dir.get().filePath(ui->listWidget_file->currentItem()->text())));ninfo<<"var:incremental_sync_set:"<<vm->incremental_sync_set;
        ui->listWidget_incremental->clear();foreach(QString i,vm->incremental_sync_set){ui->listWidget_incremental->addItem(i);show_dir();}});
    connect(ui->actionIncremental_sync_disable,&QAction::triggered,this,[vm,this]{
        vm->incremental_sync_set.remove(QDir("files").relativeFilePath(vm->o_current_dir.get().filePath(ui->listWidget_file->currentItem()->text())));ninfo<<"var:incremental_sync_set:"<<vm->incremental_sync_set;
        ui->listWidget_incremental->clear();foreach(QString i,vm->incremental_sync_set){ui->listWidget_incremental->addItem(i);}if(vm->incremental_sync_set.empty())ui->listWidget_incremental->addItem("空 （所有文件都将被同步）");show_dir();});  
    connect(ui->pushButton_file_syncAll,&QPushButton::clicked,this,[vm,this]{vm->incremental_sync_set.clear();ui->listWidget_incremental->clear();ui->listWidget_incremental->addItem("空 （所有文件都将被同步）");show_dir();});
    connect(ui->pushButton_file_addSchedule,&QPushButton::clicked,this,[this]{dialog_schedule=new Dialog_schedule(this);
        connect(dialog_schedule,&Dialog_schedule::saved,this,[this](Schedule *s){schedule_list.append(s);s->setInterval(5000);s->setEnabled(true);s->setAutoTrigger(true);
                                                                                 connect(s,&Schedule::triggered,this,[this]{ui->actionupload_file->trigger();});});
        dialog_schedule->exec();dialog_schedule->deleteLater();
        ui->listWidget_schedule->clear();foreach(auto i,schedule_list)ui->listWidget_schedule->addItem(i->toString());
        QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();
    });
    connect(ui->pushButton_file_deleteCurrentSchedule,&QPushButton::clicked,this,[this]{int i=ui->listWidget_schedule->currentRow();if(i==-1){QMessageBox::warning(this,tr("删除"),tr("请先选中一个日程"));return;}
        schedule_list[i]->deleteLater();schedule_list.removeAt(i);ui->listWidget_schedule->clear();foreach(auto i,schedule_list)ui->listWidget_schedule->addItem(i->toString());
        QFile f("config/schedule.dat");f.open(QFile::WriteOnly);QDataStream d(&f);foreach(auto i,schedule_list)d<<(*i);f.close();});
    connect(qApp,&QApplication::applicationStateChanged,this,[vm,this](Qt::ApplicationState st){androidRun{
            if(st == Qt::ApplicationSuspended){ninfo<<"应用程序退后台，自动关闭";vm->on_suspended();}
            else if(st==Qt::ApplicationActive){Utils::restart();}}});
    connect(ui->actionExit_Application,&QAction::triggered,this,&MainWindow::close);
    connect(ui->pushButton_settings_console,&QPushButton::clicked,this,[this]{if(!QDir("tools/").exists())QDir().mkpath("tools/");QFile::copy(":/rc/bin/Alacritty.exe","tools/Alacritty.exe");QProcess::startDetached("tools/Alacritty.exe",QStringList()<<"-e"<<QApplication::applicationFilePath()<<QApplication::arguments()<<"CON_MODE");close();});
    connect(ui->commandLinkButton_route_page2,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(2);});
    connect(ui->commandLinkButton_route_page3,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(3);});
    connect(ui->commandLinkButton_route_page5,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(5);});
    connect(ui->commandLinkButton_route_page6,&QCommandLinkButton::clicked,this,[this]{ui->tabWidget->setCurrentIndex(6);});
    connect(ui->pushButton_settings_mode_normal,&QPushButton::clicked,this,[this]{if(QMessageBox::warning(this,"高级设置-普通模式","这是高级设置，如果您不理解其中的含义，请不要设置。\n\n设置成普通模式后，设备将不会作为DFHN节点，无法在设备列表里便捷标记，因此无法从DFHN节点取下挂起的文件\n\n你确定要修改吗？",QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            QDir().rmdir("config/empty/label3");QProcess::startDetached(QApplication::applicationFilePath());close();
        }});
    connect(ui->pushButton_settings_mode_dfhn,&QPushButton::clicked,this,[this]{if(QMessageBox::warning(this,"高级设置-DFHN模式","这是高级设置，如果您不理解其中的含义，请不要设置。\n\n设置成DFHN模式后，设备将作为DFHN节点，在设备列表里特殊标记，用于将设备作为DFHN节点挂起文件。\n设置成DFHN模式后，可能会影响正常使用完整功能。\n\n你确定要修改吗？",QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok){
            QDir().mkpath("config/empty/label3");QProcess::startDetached(QApplication::applicationFilePath());close();
        }});
    connect(ui->actionHang_up_file_to_dfhn,&QAction::triggered,this,[vm,this]{vm->on_hangup_to_dfhn();});
    connect(ui->actionDownload_file_from_dfhn,&QAction::triggered,this,[vm,this]{vm->on_download_from_dfhn();});
    connect(ui->commandLinkButton_route_help,&QCommandLinkButton::clicked,this,[]{Dialog_help *h = new Dialog_help;h->exec();h->deleteLater();});
    connect(&timer_refresh,&QTimer::timeout,this,[this]{show_dir();});
//    connect(ui->checkBox_file_autoSync,&QCheckBox::stateChanged,this,[this](int state){is_autoSync = (state==Qt::Checked);   settings.setValue("ApplicationSettings/isAutoSync",is_autoSync);});
//    connect(&timer_autoSync,&QTimer::timeout,this,[this]{
//        if(!is_autoSync)return;
//        auto newFileHashMap = generateFileHashMap(syncFolder);
//        //比较
//        bool flag=false;
//        for(auto it = newFileHashMap.constBegin();it!=newFileHashMap.end();it++){
//            if (!fileHashMap.contains(it.key()) || it.value() != fileHashMap.value(it.key())) {
//                flag = true;
//                break;
//            }
//        }
//        if(flag==true){
//            ninfo<<"检测到文件修改，自动同步";
//            fileHashMap=newFileHashMap;
//            sendFile(lastSyncDst);
//        }
//        else{
            
//        }
//    });
    connect(ui->actionRestart_all_applications,&QAction::triggered,this,[vm,this]{vm->on_restart_all();});
    connect(vm,&ViewModel::sendInfoChanged,this,&MainWindow::on_sendInfo_updated);
    connect(vm,&ViewModel::tempMessageChanged,this,[this](QString msg,int maxtime){ui->statusBar->showMessage(msg,maxtime);});
    connect(ui->actionrestart,&QAction::triggered,this,[]{
        Utils::restart();});
    // connect(ui->pushButton_debug1,&QPushButton::clicked,this,&MainWindow::on_pushButton_debug1_clicked);
    connect(vm,&ViewModel::remoteControlEngineUpdated,this,[this](RemoteControlEngine *eng){
        ui->widget_remoteControl->deleteLater();
        QGridLayout *layout = (QGridLayout*)(ui->tab_remoteControl->layout());
        layout->addWidget(new RemoteControlWidget(this,eng),1,0);
    });
    connect(ui->actionStart_remote,&QAction::triggered,this,[this]{int index=ui->tableWidget_deviceList->currentRow();if(index==-1){QMessageBox::warning(this,"远程控制","请先选中一个设备");return;} ui->tabWidget->setCurrentIndex(2); this->vm->on_start_remote(index);});
    connect(ui->pushButton_remote_stop,&QPushButton::clicked,this,[this]{this->vm->on_stop_remote();});
    
    //其他
    vm->o_current_dir=QDir("files");
    
    //obs区
    vm->o_current_dir.use(this,[=]{show_dir();});
    vm->o_status.use(label_status,[=]{label_status->setText(vm->o_status);});
    vm->o_clients.use(this,[=]{//设备列表更新
        ui->tableWidget_deviceList->clearContents();
        ui->tableWidget_deviceList->setRowCount(0);
        auto clients=vm->o_clients.get();
        vm->o_status="设备列表更新成功";
        foreach(auto client,clients){
            int row_index=ui->tableWidget_deviceList->rowCount();
            ui->tableWidget_deviceList->insertRow(row_index);
            ui->tableWidget_deviceList->setItem(row_index,0,new QTableWidgetItem(QString("(Current)")+(client.flag==Communication::DFHNDevice?"(DFHN)":"")));
            ui->tableWidget_deviceList->setItem(row_index,1,new QTableWidgetItem((client.flag==Communication::DFHNDevice?"**":"")+client.ip));
            ui->tableWidget_deviceList->setItem(row_index,2,new QTableWidgetItem(QString::number(client.port)));
            ui->tableWidget_deviceList->setItem(row_index,3,new QTableWidgetItem((client.flag==Communication::DFHNDevice?"**":"")+client.description+"("+QMetaEnum::fromType<Communication::DeviceFlag>().valueToKey(client.flag)+")"));
        }
    });
    vm->o_user_name.use(this,[=]{ui->lineEdit_settings_username->setText(vm->o_user_name);});
    vm->o_pwd.use(this,[=]{ui->lineEdit_settings_pwd->setText(vm->o_pwd);});
    vm->o_description.use(this,[=]{ui->lineEdit_settings_description->setText(vm->o_description);});
    vm->o_disableNoticeState.use(this,[=]{ui->checkBox_settings_disableNotice->setChecked(vm->o_disableNoticeState);});
    vm->o_ipv6UsageState.use(this,[=]{ui->checkBox_settings_ipv6->setChecked(vm->o_ipv6UsageState);});
    ninfo<<"当前工作目录："<<QDir::currentPath();
    
    //安卓全屏策略
    androidComp( QTimer::singleShot(10,this,[this]{
                    setMaximumSize(QApplication::primaryScreen()->availableGeometry().size());/*QString str;QDebug(&str)<<"maxsize"<<maximumSize()<<"DPI"<< QGuiApplication::primaryScreen()->physicalDotsPerInch();QMessageBox::information(this,"",str);*/
                    ui->centralwidget->setMaximumSize(maximumSize());
                    ui->tabWidget->setMaximumSize(maximumSize());
                    ui->centralwidget->resize(maximumSize());
                    ui->tabWidget->resize(maximumSize());
                    int width = ui->tabWidget->size().width()/4;
                    //            ndb<<"width"<<width;
                    //            ninfo<<width;
                    ui->tabWidget->tabBar()->setStyleSheet(/*ui->tabWidget->tabBar()->styleSheet()+*/QString("QTabBar::tab:enabled{ width: %1px; margin: 0px; padding: 0px; }").arg(width));
                    ui->tabWidget->tabBar()->setExpanding(true);
                    ui->tabWidget->setTabPosition(QTabWidget::South);
                    hideTab(ui->tabWidget,2);
                    hideTab(ui->tabWidget,3);
                    hideTab(ui->tabWidget,5);
                    hideTab(ui->tabWidget,6);
    }); )
    winComp({
        timeBeginPeriod(1);
    });
}


MainWindow::~MainWindow(){
    
}


void MainWindow::show_dir(){        //显示目录
    ui->listWidget_file->clear();
//    vm->o_current_dir.refresh();
//    ndb<<"var:current_dir"<<current_dir.absolutePath()<<"   exists"<<current_dir.exists();
    QFileInfoList fil = vm->o_current_dir.get().entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsFirst);
//    ndb<<"list size:"<<fil.size();
    
    //添加上级目录
    if(vm->o_current_dir.get() != QDir("files")){//不能更往前了
//        ndb<<"PRE";
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(tr("(上级目录)"));
        item->setData(Qt::UserRole,QVariant("pre"));//设置标签
        ui->listWidget_file->addItem(item);
    }

    
    //遍历并输出
    bool sync_all = vm->incremental_sync_set.empty();
    foreach(auto info,fil){
//        ndb<<"for";
        if(info.isDir()){
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(info.fileName());
            item->setData(Qt::UserRole,QVariant("folder"));//设置标签
//            item->setForeground(QBrush(QColor(100,0,0)));
            item->setIcon(QFileIconProvider().icon(QFileIconProvider::Folder));
            ui->listWidget_file->addItem(item);
        }
        else{
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(info.fileName());
            item->setData(Qt::UserRole,QVariant("file"));//设置标签
            item->setIcon(QFileIconProvider().icon(info));
            if(!sync_all){
                if(vm->incremental_sync_set.contains(QDir("files/").relativeFilePath(info.filePath()))){
                    item->setForeground(QBrush(QColor(0,249,26)));
                }
                else{
                    item->setForeground(QBrush(QColor(255,147,0)));
                }
            }
            ui->listWidget_file->addItem(item);
        }
    }
}


void MainWindow::sendFile(QList<device> dst){
    if(dst.empty()){//让用户选择
        Dialog_selectSyncDst *dialog = new Dialog_selectSyncDst(this);
        auto tmp = vm->o_clients.get();
        dialog->setup(tmp);
        connect(dialog,&Dialog_selectSyncDst::syncdstDecided,this,[&](QList<device> a){dst=a;});
        dialog->exec();
        dialog->deleteLater();
        ninfo<<"SyncDst:"<<dst;
    }
    if(dst.empty()){
        QMessageBox::warning(this,tr("同步文件"),tr("请选择目标！"));
        return;
    }
//    dst.removeAll(public_ip);//文件不发给自己
//    lastSyncDst=dst;
    vm->sendFile(dst);
}


void MainWindow::restartDebug(){
    if(is_debug){
        ui->tabWidget->setTabEnabled(6,true);//启用调试页面   //发布设置
        ui->commandLinkButton_route_page6->setEnabled(true);
        ui->pushButton_switchProxy->setText("开始加速");
    }
    else{
        ui->tabWidget->setTabEnabled(6,false);//禁用调试页面   //发布设置
        ui->commandLinkButton_route_page6->setEnabled(false);
        ui->pushButton_switchProxy->setText("开始加速（大陆地区禁止使用）");
    }
}


void MainWindow::hideTab(QTabWidget* tabWidget, int index){
    QTabBar* bar = tabWidget->tabBar();
    // 禁用标签页交互
    bar->setTabEnabled(index, false);
    // 清除可见内容
    bar->setTabText(index, "");
    bar->setTabIcon(index, QIcon());
    // 设置最小尺寸为0（防止留空）
    bar->setStyleSheet( bar->styleSheet()+
        "QTabBar::tab:disabled {"
        "    min-width: 0px;"
        "    max-width: 0px;"
        "    height: 0px;"
        "    padding: 0px;"
        "    margin: 0px;"
        "}"
                        );
}


void MainWindow::on_folder_change(){
    QListWidgetItem currentItem = *ui->listWidget_file->currentItem();
    
    if(currentItem.data(Qt::UserRole) == "folder"){//如果是目录就进入
        vm->o_current_dir = QDir(vm->o_current_dir.get().filePath(currentItem.text()));
        vm->on_folder_change(vm->o_current_dir);
        show_dir();//刷新列表
    }
    if(currentItem.data(Qt::UserRole) == "pre"){//上级目录
        auto tmp=QDir(vm->o_current_dir);
        tmp.cdUp();
        vm->o_current_dir=tmp;
        vm->on_folder_change(vm->o_current_dir);
        show_dir();//刷新列表
    }
    if(currentItem.data(Qt::UserRole) == "file"){//是文件就打开
        //        ndb<<QFile(QDir(current_dir.filePath(currentItem.text())).absolutePath()).exists()<<"    "<<QDir(current_dir.filePath(currentItem.text())).absolutePath();
        //        QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators(QDir(current_dir.filePath(currentItem.text())).absolutePath())));
        QDesktopServices::openUrl(QUrl::fromLocalFile((vm->o_current_dir.get().filePath(currentItem.text()))));
    }
}


void MainWindow::on_rightclick(){       //右键点击事件
//    ndb<<"click";
//    ndb<<mapFromGlobal(QCursor::pos());
    QMenu *rightMenu = new QMenu(this);
    if(ui->listWidget_file->currentItem() != nullptr){//有Item的时候再菜单"打开"
//        ndb<<"ITEM";
        rightMenu->addAction(ui->actionOpen);
        
        if(ui->listWidget_file->currentItem()->data(Qt::UserRole) != "folder"){//目录不需要
            rightMenu->addAction(ui->actionFolder);
            if(vm->incremental_sync_set.contains(QDir("files").relativeFilePath(vm->o_current_dir.get().filePath(ui->listWidget_file->currentItem()->text())))){
                rightMenu->addAction(ui->actionIncremental_sync_disable);
            }
            else{
                rightMenu->addAction(ui->actionIncremental_sync_enable);
            }
        }
    }
    else{
        rightMenu->addAction(ui->actionFolder);
    }
    rightMenu->addAction(ui->actionRefresh);
    
    rightMenu->setAttribute(Qt::WA_DeleteOnClose);
    rightMenu->exec(QCursor::pos());
}


void MainWindow::on_settings_saved(){
    vm->on_settings_saved(
                ui->lineEdit_settings_username->text(),
                ui->lineEdit_settings_pwd->text(),
                ui->lineEdit_settings_mqttServer->text(),
                ui->spinBox_settings_mqttPort->value(),
                ui->lineEdit_settings_gitubUser->text(),
                ui->lineEdit_settings_githubPAT->text(),
                ui->comboBox_settings_uiskin->currentIndex(),
                ui->checkBox_settings_recordLog->isChecked(),
                ui->checkBox_settings_disableNotice->isChecked(),
                ui->lineEdit_settings_description->text()
    );
}


void MainWindow::on_rightclick_deviceList(){
    QMenu *rightMenu = new QMenu;
    rightMenu->setAttribute(Qt::WA_DeleteOnClose);
    rightMenu->addAction(ui->actionShutdown_current);
    rightMenu->addAction(ui->actionTest_RTT);
    rightMenu->addAction(ui->actionRequestFile);
    rightMenu->addAction(ui->actionRemoteCopyFile);
    rightMenu->addAction(ui->actionSend_message);
    rightMenu->addAction(ui->actionStart_remote);
    rightMenu->exec(QCursor::pos());
}


void MainWindow::on_test_rtt(){
    vm->on_test_rtt();
}


void MainWindow::on_sendInfo_updated(TransmissionEngine::SendInfo info){
    ui->tabWidget->setCurrentIndex(1);
    vm->o_status="正在发送";
    ui->label_sendInfo_currentPackage->setNum(info.i);
    ui->label_sendInfo_totalPackege->setNum(info.total);
    ui->label_sendInfo_currentDelay->setNum(info.delay);
    ui->label_sendInfo_currentReqAckLoop->setNum(info.reqAckLoop);
    ui->label_sendInfo_progress->setText(info.total!=0?(QString("%1%").arg(100.*info.i/info.total)):"--");
}


void MainWindow::on_pushButton_debug1_clicked(){
    ndb<<"开始调试";    
    vm->on_debug();
    // connect(vm,&ViewModel::debugSignal,this,[this](QVariantMap m){ui->label_remote_screen->setPixmap(QPixmap::fromImage(m["data"].value<QImage>()).scaled(ui->label_remote_screen->size()));});
}

//bool MainWindow::sendReliableMessage(int dst, QString msg){
//    //生成唯一ID
//    QUuid uuid = QUuid::createUuid();
//    //发送消息-1
//    for(int i=0;i<3;i++){
//        QJsonObject json1;
//        json1.insert("reliable_msg","DATA");
//        json1.insert("uuid",uuid.toString(QUuid::Id128));
//        json1.insert("value",msg);
//        send(QJsonDocument(json1).toJson(),1,dst);
//        //等待回复
//        QEventLoop loop;
//        bool flag1=false;
//        QTimer::singleShot(3000,&loop,&QEventLoop::quit);
//        connect(m_communication,&Communication::readyRead,&loop,&QEventLoop::quit);
//        connect(m_communication,&Communication::readyRead,this,[&]{flag1=true;});
//        loop.exec();
//        if(flag1&&lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())&&lastMessage.contains("R_ACK_DATA"))break;
//        if(i==2)return false;
//    }
//    //发送允许释放
//    for(int i=0;i<3;i++){
//        QJsonObject json1;
//        json1.insert("reliable_msg","ALO_RLS");
//        json1.insert("uuid",uuid.toString(QUuid::Id128));
//        send(QJsonDocument(json1).toJson(),1,dst);
//        //等待回复
//        QEventLoop loop;
//        bool flag1=false;
//        QTimer::singleShot(3000,&loop,&QEventLoop::quit);
//        connect(m_communication,&Communication::readyRead,&loop,&QEventLoop::quit);
//        connect(m_communication,&Communication::readyRead,this,[&]{flag1=true;});
//        loop.exec();
//        if(flag1&&lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())&&lastMessage.contains("R_ACK_RLS"))break;
//        if(i==2)return false;
//    }
//    return true;
//}


//QVector<QVector<QPair<ipport, ipport>>> MainWindow::planAutoSend(QList<device> dsts){
//    QQueue<ipport> senders;
//    QQueue<ipport> receivers;
//    QVector<QVector<QPair<ipport, ipport>>> result;
//    foreach(ipport i,dsts)receivers.append(i);
//    senders.append(public_ip);
    
//    //开始规划
//    ninfo<<"开始规划发送表";
//    while(!receivers.empty()){//规划到没有可用的接收者了
//        QQueue<ipport> new_senders;
//        QVector<QPair<ipport, ipport>> thisRound;
//        ninfo<<"----------";
//        foreach(ipport sender , senders){
//            if(receivers.empty())break;
//            thisRound.append(QPair<ipport,ipport>(sender,receivers.front()));
//            ninfo<<"配对："<<sender<<" "<<receivers.front();
//            new_senders.append(receivers.front());
//            receivers.pop_front();
//        }
//        senders.append(new_senders);
//        result.append(thisRound);
//    }
//    ninfo<<"结果："<<(result);
//    return result;
//}

//QMap<QString, QByteArray> MainWindow::generateFileHashMap(QDir baseDir){
//    /*QMap<QString,QByteArray> result;
//    QFileInfoList infoList = baseDir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
//    foreach(auto info,infoList){
//        if(info.isFile()){
//            QFile f(info.absoluteFilePath());
//            if(f.open(QFile::ReadOnly)){
//                QCryptographicHash hash(QCryptographicHash::Md5);
//                hash.addData(&f); 
//                f.close();
//                result.insert(info.absoluteFilePath(),hash.result());
//            }
//            else{
//                ncritical<<"文件打开失败！File:"<<f<<" Info:"<<f.errorString();
//            }
//        }
//        if(info.isDir()){
//            result.unite(generateFileHashMap(QDir(info.absoluteFilePath())));
//        }
//    }
//    return result;*/
//    QMap<QString,QByteArray> result;
//    auto infoList = traverseFolder(baseDir);
//    foreach(auto ifp,infoList){
//        auto info = ifp.first;
//        if(info.isFile()){
//            QFile f(info.absoluteFilePath());
//            if(f.open(QFile::ReadOnly)){
//                QCryptographicHash hash(QCryptographicHash::Md5);
//                hash.addData(&f); 
//                f.close();
//                result.insert(info.absoluteFilePath(),hash.result());
//            }
//            else{
//                ncritical<<"文件打开失败！File:"<<f<<" Info:"<<f.errorString();
//            }
//        }
//        if(info.isDir()){
//            //什么也不做
//        }
//    }
//    return result;
//}


//void MainWindow::initNetwork(function<void(QString)> func_update){
//    //网络部分
//    //发起STUN
//    if(QDir("config/empty/label3").exists())show();
//    label_status->setText(tr("正在获取公网IP……"));
//    if(func_update)func_update(tr("正在获取公网IP……"));
//    if(QDir("config/empty/label3").exists())QCoreApplication::processEvents();
//    public_ip = m_communication->stun();
//    public_ip.description=device_description;
//    public_ip.flag = device_flag;
//    if(public_ip == Communication::ipport{"",0} || QApplication::arguments().contains("-ipv6")){
//        QCoreApplication::processEvents();
        
//        //开始用IPv6
//        public_ip = m_communication->getIPv6();
        
//        if(public_ip == Communication::ipport{"",0}){
//            QMessageBox::critical(this,tr("获取公网IP"),tr("您的设备获取IPv4公网IP失败，且不支持IPV6。\n请如果是第一次发生，请重试。\n如果多次发生，请只使用文件挂起功能或更换网络环境。"));
////            QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());
////            QCoreApplication::quit();
////            abort();
////            QApplication::processEvents();
//        }
//    }
//    label_status->setText(tr("正在获取上线用户列表……"));
//    if(func_update)func_update(tr("正在获取上线用户列表……"));
//    ninfo<<"var:public ip="<<public_ip;
//    //MQTT
//    m_signalling->connectToHost(/*{"broker.emqx.io",1883}*/mqtt_server);
//    m_signalling->subscribe("nnpyro_syncTunnel/user_topics/" + user_name.toUtf8().toBase64());
//    m_signalling->setPwd(pwd.toUtf8());
//    m_signalling->setUser(public_ip,user_name);
//    clients = m_signalling->getUserList();//获取用户列表
//    if(1)foreach(auto i,clients)qDebug()<<i;
//    label_status->setText(tr("加载成功"));
//    if(func_update)func_update(tr("加载成功"));
    
//    //等待直到用户列表获取完成
//    QEventLoop el1;
//    connect(m_signalling,&Signalling::on_userlist_updata,&el1,&QEventLoop::quit);
////    el1.exec(QEventLoop::ExcludeUserInputEvents);
////    QCoreApplication::processEvents();
    
//    //调试
//    if(1)ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));
//    if(1)foreach(auto i,clients)ui->textBrowser_debug1->append(i);
    
//    //设置文件挂起
//    m_storage->setUser(user_github_name,user_github_PAT);
//    m_storage->setUserId(user_name);
    
//    //接管communicaion
//    ndb<<"接管communication";
//    m_transmissionengine = new TransmissionEngine(m_communication,user_name,pwd,public_ip,this);
//    m_transmissionengine->setClients(clients);
//    connect(m_transmissionengine,&TransmissionEngine::communicationReadyRead,this,&MainWindow::on_readyRead);
//    connect(m_transmissionengine,&TransmissionEngine::SPTP_readyRead,this,&MainWindow::on_SPTP_readyRead);
//    connect(m_transmissionengine,&TransmissionEngine::messageChanged,this,[this](QString msg){label_status->setText(msg);});
//    connect(m_transmissionengine,&TransmissionEngine::SPTP_sendFinished,this,[this]{label_status->setText("发送成功");playSound(QUrl("qrc:/rc/audio/file_send_successfully.wav"));});
//    connect(m_transmissionengine,&TransmissionEngine::SPTP_ctrlMsgReceived,this,&MainWindow::on_SPTP_ctrlMsg_received);
//    connect(m_signalling,&Signalling::on_userlist_updata,this,[this](QList<Communication::device> userl){
//        clients = userl;if(1)foreach(auto i,clients)qDebug()<<i;
//        QTimer::singleShot(3000,[this]{
//            for(int i=0;i<5;i++){
//                send("{\n    \"hole\":1\n}");
//                QThread::msleep(20);
//                send("{\n    \"hole\":2\n}");
//                QThread::msleep(20);
//            }
//        });
//        label_status->setText(tr("用户列表更新成功"));
//        ui->tableWidget_deviceList->clearContents();ui->tableWidget_deviceList->setRowCount(0);ndb<<"IN";
//        if(m_transmissionengine)m_transmissionengine->setClients(clients);
        
//        foreach(auto i,clients){        //表格设置
//            int r=ui->tableWidget_deviceList->rowCount();
//            ui->tableWidget_deviceList->insertRow(r);
//            ui->tableWidget_deviceList->setItem(r,0,new QTableWidgetItem((i.flag==Communication::DFHNDevice?"**":"") + user_name));
//            ui->tableWidget_deviceList->setItem(r,1,new QTableWidgetItem(i.ip));
//            ui->tableWidget_deviceList->setItem(r,2,new QTableWidgetItem(QString::number(i.port)));
//            ui->tableWidget_deviceList->setItem(r,3,new QTableWidgetItem((i.flag==Communication::DFHNDevice?"**":"")+i.description+"("+QMetaEnum::fromType<Communication::DeviceFlag>().valueToKey(i.flag)+")"));
//        }
//        if(1){ui->textBrowser_debug1->clear();ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));foreach(auto i,clients)ui->textBrowser_debug1->append(i.toFullString());}
//    });
    
//    //打洞
//    for(int i=0;i<5;i++){
//        send("{\n    \"hole\":1\n}");
//        send("{\n    \"hole\":2\n}");
//        QThread::msleep(50);
//    }
//    label_status->setText(tr("加载成功"));
//}

//void MainWindow::restartNetwork(){
//    if(m_transmissionengine)m_transmissionengine->deleteLater();
//    if(m_signalling)m_signalling->deleteLater();
//    if(m_communication)m_communication->deleteLater();
//    QApplication::processEvents(QEventLoop::AllEvents,100);
//    QThread::sleep(1);
//    initNetwork();
//}


//QList<QPair<QFileInfo,QDir>> MainWindow::traverseFolder(QDir folder){
//    QList<QPair<QFileInfo,QDir>> ret;
//    QFileInfoList info = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsLast);
////    ninfo<<"进入目录"<<folder.absolutePath()<<"长度"<<info.size();
//    foreach(QFileInfo fi ,info){
//        if(fi.isFile()){
////            ninfo<<"处理文件"<<fi.absoluteFilePath();
//            if(fi.fileName().endsWith(".private.stlink")){
//                //处理符号链接
//                QFile f(fi.absoluteFilePath());
//                f.open(QFile::ReadOnly);
//                QString dir = f.readAll().trimmed();
//                f.close();
//                if(QFile(dir).exists()){
//                    QFileInfo info(dir);
//                    QDir logicalDir = QDir(fi.absolutePath()).absoluteFilePath(QFileInfo(dir).fileName());
//                    ret.append(qMakePair(info,logicalDir));
//                }
//            }
//            else{
//                ret.append(qMakePair(fi,QDir(fi.absoluteFilePath())));
//            }
//        }
//        else{//目录递归DFS
////            ninfo<<"处理目录"<<fi.absoluteFilePath();
//            ret.append(qMakePair(fi,QDir(fi.absoluteFilePath())));
//            ret<<(traverseFolder(QDir(fi.absoluteFilePath())));
//        }
//    }
//    return ret;
//}


//void MainWindow::on_folder_change(){
//    QListWidgetItem currentItem = *ui->listWidget_file->currentItem();
    
//    if(currentItem.data(Qt::UserRole) == "folder"){//如果是目录就进入
//        current_dir = QDir(current_dir.filePath(currentItem.text()));
//        show_dir();//刷新列表
//    }
//    if(currentItem.data(Qt::UserRole) == "pre"){//上级目录
//        current_dir.cdUp();
//        show_dir();//刷新列表
//    }
//    if(currentItem.data(Qt::UserRole) == "file"){//是文件就打开
////        ndb<<QFile(QDir(current_dir.filePath(currentItem.text())).absolutePath()).exists()<<"    "<<QDir(current_dir.filePath(currentItem.text())).absolutePath();
////        QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators(QDir(current_dir.filePath(currentItem.text())).absolutePath())));
//        QDesktopServices::openUrl(QUrl::fromLocalFile((current_dir.filePath(currentItem.text()))));
//    }
//}


//void MainWindow::on_rightclick(){       //右键点击事件
////    ndb<<"click";
////    ndb<<mapFromGlobal(QCursor::pos());
//    QMenu *rightMenu = new QMenu(this);
//    if(ui->listWidget_file->currentItem() != nullptr){//有Item的时候再菜单"打开"
////        ndb<<"ITEM";
//        rightMenu->addAction(ui->actionOpen);
        
//        if(ui->listWidget_file->currentItem()->data(Qt::UserRole) != "folder"){//目录不需要
//            rightMenu->addAction(ui->actionFolder);
//            if(incremental_sync_set.contains(QDir("files").relativeFilePath(current_dir.filePath(ui->listWidget_file->currentItem()->text())))){
//                rightMenu->addAction(ui->actionIncremental_sync_disable);
//            }
//            else{
//                rightMenu->addAction(ui->actionIncremental_sync_enable);
//            }
//        }
//    }
//    else{
//        rightMenu->addAction(ui->actionFolder);
//    }
//    rightMenu->addAction(ui->actionRefresh);
    
//    rightMenu->setAttribute(Qt::WA_DeleteOnClose);
//    rightMenu->exec(QCursor::pos());
//}


//void MainWindow::on_readyRead(QByteArray msg){
////    while(m_communication->hasPendingDatagrams() || !currentReliableMsg.isEmpty()){
////    bool penDingDatagramFlag = m_communication->hasPendingDatagrams();
////    QNetworkDatagram datagram = m_communication->readDatagram();
//////    ipport sender = {datagram.senderAddress().toString(),(quint16)datagram.senderPort()}; //###
//////    ndb<<"data"<<datagram.data();
////    //解密数据
////    QByteArray msg;
////    if(penDingDatagramFlag){
////        msg = decode(datagram.data());
////        lastMessage=msg;
////        //    ndb<<"var:msg"<<msg;
////        QJsonObject json_temp = QJsonDocument::fromJson(msg).object();json_temp.remove("filebody");
////        ninfo<<"var:msg(no filebody)"<<QJsonDocument(json_temp).toJson();
////    }
////    else if(!currentReliableMsg.isEmpty()){
////        msg=currentReliableMsg.toUtf8();
////        currentReliableMsg.clear();
////        ninfo<<"var:msg(relieableMsg)"<<msg;
////    }
    
//    QJsonDocument jd = QJsonDocument::fromJson(msg);
//    QJsonObject json;
    
//    if(!jd.isObject()){
//        if(!msg.startsWith("FB")&&!msg.startsWith("BF"))ninfo<<"var:msg = "<<msg;
//        if(msg == "FILE_RELEASE_SUCCESSFULLY" && !chunks.empty()){
//            label_status->setText(tr("文件发送可能成功"));
//        }
//        else if(msg == "REQ_FILE"){
//            if(timer_is_uploading.isActive()){
//                QEventLoop loop;
//                connect(&timer_is_uploading,&QTimer::timeout,&loop,&QEventLoop::quit);
//                loop.exec();
//            }
//            else QThread::msleep(500);
//            QMetaObject::invokeMethod(this,[=]{sendFile(clients);},Qt::QueuedConnection);
//        }
//        else if(msg == "KEEP_ALIVE"){
            
//        }
//        else if(msg.startsWith("REMOTE_IMG_1_")){
//            if(!ui->widget_remote->isEnabled())ui->widget_remote->setEnabled(true);
//            QByteArray a = msg.mid(13);
//            QPixmap dst;
//            if(ui->label_remote_screen->pixmap() == nullptr || ui->label_remote_screen->pixmap()->isNull()){
//                dst=QPixmap(ui->label_remote_screen->size());
//                dst.fill(Qt::transparent);
//            }
//            else dst = *ui->label_remote_screen->pixmap();
//            QImage rc_img=QImage::fromData(a);
//            rc_img=rc_img.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QImage im=dst.toImage();
////            im=im.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QPainter painter(&im);
//            painter.drawImage(0,0,rc_img);
//            painter.end();
//            ui->label_remote_screen->setPixmap(QPixmap::fromImage(im));
//        }
//        else if(msg.startsWith("REMOTE_IMG_2_")){
//            if(!ui->widget_remote->isEnabled())ui->widget_remote->setEnabled(true);
//            QByteArray a = msg.mid(13);
//            QPixmap dst;
//            if(ui->label_remote_screen->pixmap() == nullptr || ui->label_remote_screen->pixmap()->isNull()){
//                dst=QPixmap(ui->label_remote_screen->size());
//                dst.fill(Qt::transparent);
//            }
//            else dst = *ui->label_remote_screen->pixmap();
//            QImage rc_img=QImage::fromData(a);
//            rc_img=rc_img.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QImage im=dst.toImage();
////            im=im.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QPainter painter(&im);
//            painter.drawImage(dst.width()/2,0,rc_img);
//            painter.end();
//            ui->label_remote_screen->setPixmap(QPixmap::fromImage(im));
//        }
//        else if(msg.startsWith("REMOTE_IMG_3_")){
//            if(!ui->widget_remote->isEnabled())ui->widget_remote->setEnabled(true);
//            QByteArray a = msg.mid(13);
//            QPixmap dst;
//            if(ui->label_remote_screen->pixmap() == nullptr || ui->label_remote_screen->pixmap()->isNull()){
//                dst=QPixmap(ui->label_remote_screen->size());
//                dst.fill(Qt::transparent);
//            }
//            else dst = *ui->label_remote_screen->pixmap();
//            QImage rc_img=QImage::fromData(a);
//            rc_img=rc_img.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QImage im=dst.toImage();
////            im=im.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QPainter painter(&im);
//            painter.drawImage(0,dst.height()/2,rc_img);
//            painter.end();
//            ui->label_remote_screen->setPixmap(QPixmap::fromImage(im));
//        }
//        else if(msg.startsWith("REMOTE_IMG_4_")){
//            if(!ui->widget_remote->isEnabled())ui->widget_remote->setEnabled(true);
//            QByteArray a = msg.mid(13);
//            QPixmap dst;
//            if(ui->label_remote_screen->pixmap() == nullptr || ui->label_remote_screen->pixmap()->isNull()){
//                dst=QPixmap(ui->label_remote_screen->size());
//                dst.fill(Qt::transparent);
//            }
//            else dst = *ui->label_remote_screen->pixmap();
//            QImage rc_img=QImage::fromData(a);
//            rc_img=rc_img.scaled(ui->label_remote_screen->width()/2,ui->label_remote_screen->height()/2);
//            QImage im=dst.toImage();
//            QPainter painter(&im);
//            painter.drawImage(dst.width()/2,dst.height()/2,rc_img);
//            painter.end();
//            ui->label_remote_screen->setPixmap(QPixmap::fromImage(im));
//        }
//        else if(msg=="STOP_REMOTE"){
//            timer_remote_sending.stop();
//            send("ACK_STOP_REMOTE",1,clients.indexOf(remote_be_device));
//        }
//        else if(msg=="ACK_STOP_REMOTE"){
//            ui->widget_remote->setEnabled(false);
//        }
//        else if(msg.startsWith("FB")||msg.startsWith("BF")){
//            if(msg.size()<(int)sizeof(header_filebody)){
//                nwarning<<"Warning:长度过小";
//            }
//            else{
//                if(!schedule_list.isEmpty() && schedule_list[0]->isEnabled()){
//                    foreach(auto s , schedule_list){//禁用日程
//                        s->setEnabled(false);
//                    }
//                }
//                header_filebody header;
//                memcpy(&header,msg.constData(),sizeof(header));
//                QByteArray fileContent = msg.mid(sizeof(header));
                
//                ninfo<<"收到包"<<header.no<<"/"<<header.total;
//                if(currentFileTotal!=header.total)currentFileMap.clear();
//#ifdef Q_OS_WIN
//                if(currentFileMap.empty())timeBeginPeriod(1);
//#endif
//                currentFileTotal = header.total;
//                receive_last_pack_index = header.no;
//                currentFileMap[header.no] = fileContent;
//                timer_fileResend.stop();
//                timer_fileResend.start(3000);
//                if(this->chunks.empty()){
//                    label_status->setText(QString(tr("正在接收：包%1/%2")).arg(header.no).arg(header.total));
//                }
//                if(currentFileMap.size() == currentFileTotal){
//                    label_status->setText(tr("文件操作成功"));
//                }
//            }
//        }
//        else if(msg.startsWith("SEND_TASK")){
//            QString task = msg.mid(9);
//            sendTask = task.split(';');
//            label_status->setText(QString("接受传输任务分配成功 任务数:%1").arg(sendTask.size()));
//            ninfo<<"传输任务："<<sendTask;
//            ui->textBrowser_debug1->append("传输任务：");
//            ui->textBrowser_debug1->append(sendTask.join(";"));
//        }
//        else if(msg=="DING"){
//            QMessageBox::information(this,"叮","有人叮了一下你");
//        }
//        else{
//            ncritical<<"ERROR:msg isn`t an object!";
//            ninfo<<QString(msg);
//            return;
//        }

//    }
//    else{
//        json = jd.object();
//    }
    
//    //基本变量创建
//    ipport sender = {
//        json["ip"].toString(),
//        static_cast<quint16>(json["port"].toInt())
//    };
//    int sender_index = clients.indexOf(sender);
    
    
//    //消息解析
//    if(json.contains("hole")){          //打洞
//        QJsonObject replyJson;
//        if(json["hole"].toInt() == 2){
////            replyJson.insert("hole",3);
////            send(QJsonDocument(json).toJson());
//            send("{\n    \"hole\":3\n}");
//            statusBar()->showMessage(QString("成功与%1建立连接").arg(sender),5000);
//        }
//        else if(json["hole"].toInt() == 3){
//            statusBar()->showMessage(QString("成功向%1进行NAT打洞").arg(sender),5000);
//        }
//        /*if(json["hole"].toInt() == 2){
//            replyJson.insert("hole",3);
//        }
//        if(json["hole"].toInt() == 3     &&0){
//            replyJson.insert("hole",4);
//        }*/
//    }
//    /*if(json.contains("filebody")){
//        if(currentFileTotal!=json["total"].toInt())currentFileMap.clear();
//#ifdef Q_OS_WIN
//        if(currentFileMap.empty())timeBeginPeriod(1);
//#endif
//        currentFileTotal = json["total"].toInt();
//        receive_last_pack_index = json["no"].toInt();
//        currentFileMap[json["no"].toInt()] = json["filebody"].toString();
//        timer_fileResend.stop();
//        timer_fileResend.start(3000);
//        if(this->chunks.empty()){
//            label_status->setText(QString("正在接收：包%1/%2").arg(json["no"].toInt()).arg(json["total"].toInt()));
//        }
//        if(currentFileMap.size() == currentFileTotal){
//            label_status->setText("文件操作成功");
//        }
        
////        //检测map连续
////        auto it = currentFileMap.constBegin();
////        int pre = -1;
////        QList<int> resendList;
////        for(;it != currentFileMap.constEnd();it++){
////            if(it.key() != pre+1){
////                for(int i=pre+1;i<it.key();i++){
////                    resendList.append(i);
////                }
////            }
////            pre=it.key();
////        }
////        if(currentFileMap.lastKey() != json["no"].toInt()){
////            for(int i=currentFileMap.lastKey()+1;i<=currentFileTotal;i++){
////                resendList.append(i);
////            }
////        }
        
////        if(resendList.size()>receive_lost_count){
////            send(QString("{\n    \"lost\":\"%1\"\n}").arg(resendList.size()).toUtf8(),1,sender_index);
////            receive_lost_count = resendList.size();
////        }
////        else{
////            send(QString("{\n    \"ack\":\" \"\n}").toUtf8(),1,sender_index);
////        }
        
//    }*/
//    if(json.contains("request_resend")/*&&is_uploading*/ && !chunks.empty()){
////        QJsonObject rpjson;
////    //    json.insert("user",this->user_name);
////        rpjson.insert("total",chunks.size()-1);
////        rpjson.insert("filebody","");
////        rpjson.insert("no",-1);
////            rpjson["filebody"] = chunks[json["request_resend"].toInt()];
////            rpjson["no"] = json["request_resend"].toInt();
////            send(QJsonDocument(rpjson).toJson());
////            timer_is_uploading.stop();
////            timer_is_uploading.start(10000);
////            label_status->setText("正在回复重传请求……");
//        header_filebody header;
//        header.check_type = mt_filebody;
//        header.total=chunks.size()-1;
//        header.no = json["request_resend"].toInt();
//        QByteArray msg_to_send;
//        msg_to_send.append(reinterpret_cast<const char *>(&header),sizeof(header));
//        msg_to_send.append(chunks[header.no]);
////        send(reinterpret_cast<const char *>(&header)+chunks[header.no]);
//        send(msg_to_send);
//        timer_is_uploading.stop();
//        timer_is_uploading.start(10000);
//        label_status->setText(tr("正在回复重传请求……"));
//    }
//    if(json.contains("pat")){
//        ui->lineEdit_settings_githubPAT->setText(json["pat"].toString());
//        ui->pushButton_settings_save->click();
//    }
//    if(json.contains("cmd")){
//#ifdef Q_OS_WIN
//        QStringList trustList;trustList<<"shutdown";
//        bool trust=0;
//        for(auto j : trustList){
//            if(json["cmd"].toString().startsWith(j))trust=1;
//        }
//        if(!trust)attackProtection(at_influential,
//                                   QString("收到不信任的远程命令'%1'").arg(json["cmd"].toString()),
//                                   "没有执行这个命令",
//                                   []{});
//        if(json["cmd"].toString().contains("shutdown")){is_accept_shutdown=true;ShutdownBlockReasonDestroy((HWND)winId());}
//        if(trust)QProcess::startDetached(json["cmd"].toString());
//#else
//        QMessageBox::information(this,"警告",QString(tr("警告：设备接收到来自远程设备'%1'的远程命令：\n\n%2\n\n这是Windows平台的特定命令，您的设备无法运行，已自动忽略。")).arg(sender).arg(json["cmd"].toString()));
//#endif
//    }
//    if(json.contains("opt")){
//        QString opt = json["opt"].toString();
        
//        if(opt == "test_if_connected"){//连通性测试
//            if(sender_index == -1){
//                label_status->setText("<font color=\"red\">错误：收到未知来源的连通性测试包("+sender+")</font>");
//            }
//            else{
////                QThread::msleep(20);
//                send("{\n    \"opt\":\"ack_test_if_connected\"\n}",1,sender_index);
//            }
//        }
//        if(opt == "ack_test_if_connected"){
//            test_if_connected_set.insert(sender);
////            if(!test_if_connected_set.contains(public_ip)) test_if_connected_set.insert(public_ip);
////            if(test_if_connected_set == QSet<ipport>(clients.begin(),clients.end())){
//            if(sender_index==currentSendDst){
//                emit signal_test_if_connected_finished({});
//            }
//        }
//        if(opt == "rtt_test"){
//            send("{\n    \"opt\":\"ack_rtt_test\"\n}",1,sender_index);
//        }
//        if(opt == "ack_rtt_test"){
//            rtt_result[sender_index] = elapsed_rtt.elapsed();
//        }
//        if(opt == "start_remote"){
//            timer_remote_sending.start(200);
//            remote_be_device = sender;
////            send("{\n    \"opt\":\"start_remote_successfully\"\n}",1,sender_index);
//        }
//    }
//    if(json.contains("lost") && !chunks.empty()){
////        send_current_delay += json["lost"].toInt() * 2;
////        send_stable_count = 0;
////        if(send_current_delay > SEND_MAX_DELAY) send_current_delay = SEND_MAX_DELAY;
////        ndb<<"发送速度调低到"<<send_current_delay;
//        if(/*json["check1"].toInt()==send_current_reqAck*/json["uuid"].toString() == send_req_ack_uuid.toString()){//必需是本轮发送的包而不是上轮延迟到达的
//            send_lost_count[sender] += QString(json["lost"].toString()).toInt();
//            QJsonArray ar = json["lost_list"].toArray();
//            foreach(auto i , ar) send_current_fastresend_map.insert(i.toInt());
//            if(send_lost_count.size() >= /*clients.size()-1*/1){
//                emit signal_reqAck_finished({});
//            }
//        }
//    }
//    if(json.contains("ack") && !chunks.empty()){//没有ack包了。取消此功能
////        send_ack_count[sender] += 1;
////        send_stable_count += 1;
////        int min = INT_MAX;
////        foreach(int i , send_ack_count){
////            if(i<min) min=i;
////        }
////        if(!(sender == public_ip)){
//////            elapsed_lase_ack.restart();
////        }
////        ndb<<"稳定计数"<<send_stable_count;
////        ndb<<"最小ack"<<min;
////        if(send_stable_count > 5 && min > 5){
////            send_current_delay -= 10;
////            send_stable_count = 0;
////            send_ack_count.clear();
////            if(send_current_delay < SEND_MIN_DELAY) send_current_delay = SEND_MIN_DELAY;
////            ndb<<"发送速度调高到"<<send_current_delay;
////        }
////        send_ack_count[sender]+=json["ack"].toInt();//增加ack计数
//    }
//    if(json.contains("req_ack")){
//        if(receive_last_uuid!=json["uuid"].toString()){
//            //检查丢包
//            int lost = 0;
//            int ack_pack = json["req_ack"].toInt();//应该收到的包的index
//            QJsonArray ar;
//            for(int i = json["start_req_ack"].toInt(); i <= ack_pack; i++){
//                if(!currentFileMap.contains(i)){
//                    lost++;
//                    ninfo<<"lost"<<i;
//                    ar.append(i);
//                }
//            }
//    //        receive_last_ack_index = ack_pack;
//            ninfo<<"丢失"<<lost;
//            QThread::usleep(2000);
//            QJsonObject rpJson;
//            rpJson.insert("lost",QString::number(lost));
//            rpJson.insert("uuid",json["uuid"].toString());
//            rpJson.insert("lost_list",ar);
//            QByteArray sendData = QJsonDocument(rpJson).toJson();
//            send(sendData,1,sender_index);//回复丢失信息
//    //        QThread::usleep(500);
//            send(sendData,1,sender_index);//回复丢失信息
//    //        QThread::usleep(30000);
//            send(sendData,1,sender_index);//三次回复丢失信息防止丢包
//    //        send(QString("{\n    \"lost\":\"%1\"\n}").arg(lost).toUtf8(),1,sender_index);//四次回复丢失信息防止丢包
//            receive_last_uuid = json["uuid"].toString();
//        }
//    }
//    if(json.contains("get_folderList")){
//        QFileInfoList infolist = QDir(json["get_folderList"].toString()).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks,QDir::DirsFirst|QDir::Name);
//        QSet<QPair<bool,QString>> list;
//        ninfo<<"size"<<infolist.size();
//        std::cerr<<"size"<<infolist.size();
//        QJsonObject json_reply;
//        for(int ii=0;ii<infolist.size();ii++){
//            auto i = infolist[ii];
//            list.insert(QPair<bool,QString>(i.isFile(),i.fileName()));
//            ninfo<<"扫描到"<<i.fileName();
//        }
//        QByteArray content;
//        QDataStream stm(&content,QIODevice::WriteOnly);
//        stm<<list.values();
//        json_reply.insert("folderList",QString(content.toBase64()));
//        json_reply.insert("folderList_folder",json["get_folderList"].toString());
//        QThread::msleep(500);
//        send(QJsonDocument(json_reply).toJson(),1,sender_index);
//    }
//    if(json.contains("folderList")){
//        QByteArray data = QByteArray::fromBase64(json["folderList"].toString().toUtf8());
//        QDataStream stm(data);
//        QList<QPair<bool, QString>> list;
//        stm >> list;
//        dialog_remoteFile->setFileFolder(json["folderList_folder"].toString(),QSet<QPair<bool,QString>>(list.cbegin(),list.cend()));
//    }
//    if(json.contains("copy")){
//        QFile::copy(json["copy"].toString(),QDir("files/").absoluteFilePath(QFileInfo(json["copy"].toString()).fileName()));
//    }
//    if(json.contains("test_msg")){
//        QMessageBox::information(this,"收到了一个测试消息",(json["test_msg"].toString()));
//        send("DING",1,sender_index);
//    }
//    if(json.contains("remote_event")){
//        QString msg=json["remote_event"].toString();
//#ifdef Q_OS_WIN
//        if(msg=="mouse_move"){
//            double mx=json["mx"].toDouble(),my=json["my"].toDouble();
//            QSize size = QApplication::primaryScreen()->size();
//            QCursor::setPos(mx*size.width(),my*size.height());
//        }
//        if(msg=="mleft"){
//            INPUT ipt={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
////            ipt.mi.dx=0;
////            ipt.mi.dy=0;
//            ipt.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
//            INPUT ipt2={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
////            ipt2.mi.dx=0;
////            ipt2.mi.dy=0;
//            ipt2.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
//            SendInput(1,&ipt,sizeof(ipt));
//            QThread::msleep(15);
//            SendInput(1,&ipt2,sizeof(ipt2));
//        }
//        if(msg=="mright"){
//            INPUT ipt={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
//            ipt.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN;
//            INPUT ipt2={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
//            ipt2.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP;
//            SendInput(1,&ipt,sizeof(ipt));
//            QThread::msleep(15);
//            SendInput(1,&ipt2,sizeof(ipt2));
//        }
//        if(msg=="mmid"){
//            INPUT ipt={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
//            ipt.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP;
//            INPUT ipt2={};
//            ipt.type=INPUT_MOUSE;
//            ipt.mi.dx=QCursor::pos().x()*65535/QApplication::primaryScreen()->size().width();
//            ipt.mi.dy=QCursor::pos().y()*65535/QApplication::primaryScreen()->size().height();
//            ipt2.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MIDDLEUP;
//            SendInput(1,&ipt,sizeof(ipt));
//            QThread::msleep(15);
//            SendInput(1,&ipt2,sizeof(ipt2));
//        }
//#else
//        QMessageBox::warning(this,"警告",QString("警告：接收到设备'%1'远程控制本机的命令'%2'。只有Windows电脑才能被远程控制，已自动忽略此命令。").arg(sender).arg(msg));
//#endif
        
//    }
//    if(json.contains("reliable_msg")){//可靠消息处理
//        QString control_msg = json["reliable_msg"].toString();
//        if(control_msg=="DATA"){
//            if(currentReliableUuid.isEmpty()){
//                currentReliableMsg=json["value"].toString();
//                currentReliableUuid=json["uuid"].toString();
//                send((QString("R_ACK_DATA")+currentReliableUuid).toUtf8(),1,sender_index);
//            }
//        }
//        if(control_msg=="ALO_RLS"){
//            if(currentReliableUuid==json["uuid"].toString()){
//                currentReliableMsg.clear();
//                QMetaObject::invokeMethod(this,/*&MainWindow::on_readyRead*/[this]{on_readyRead(currentReliableMsg.toUtf8());},Qt::QueuedConnection);
//            }
//            send((QString("R_ACK_RLS")+currentReliableUuid).toUtf8(),1,sender_index);
//        }
//    }
////    }
//}


//void MainWindow::on_SPTP_readyRead(QByteArray msg){
//    releaseFile(msg);
//    playSound(QUrl("qrc:/rc/audio/file_release_successfully.wav"));
//}


//void MainWindow::on_request_resend(){
//    //检测map连续 这个检测连续算法已经测试，没有问题
//    auto it = currentFileMap.constBegin();
//    int pre = -1;
////    it++;
//    QList<int> resendList;
//    for(;it != currentFileMap.constEnd();it++){
//        if(it.key() != pre+1){
//            for(int i=pre+1;i<it.key();i++){
//                resendList.append(i);
//            }
//        }
//        pre=it.key();
//    }
//    if(currentFileMap.lastKey() != currentFileTotal){
//        for(int i=currentFileMap.lastKey()+1;i<=currentFileTotal;i++){
//            resendList.append(i);
//        }
//    }
    
//    ninfo<<"var:list(丢包) ="<<resendList;
    
    
//    //请求重传
//    QJsonObject json;
//    json.insert("request_resend",0);
//    foreach(auto i , resendList){
//        json["request_resend"] = i;
//        send(QJsonDocument(json).toJson());
//        label_status->setText(QString(tr("正在请求重传%1个包")).arg(resendList.size()));
//        QThread::msleep(100);
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents,100);
//    }
    
//    timer_fileResend.stop();
//    if(resendList.size() != 0){//如果重新传输了那么就不能执行后面的合并文件否则会合并一个错误的文件
//        timer_fileResend.start(3000); // 3秒后再次检查文件完整性
//        return;
//    }
    
//    timer_clear_currentFileMap.stop();
//    timer_clear_currentFileMap.start(10000);
    
//    QEventLoop loop;        //定义一个新的事件循环
//    QTimer::singleShot(500, &loop,&QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
//    loop.exec(); 
    
//    //合并文件
//    QByteArray n;
//    foreach(QByteArray i,currentFileMap){
//        n.append(i);
//    }
////    ui->textBrowser_debug1->setText(n);
//    ninfo<<"Log:文件请求已完成，字节数："<<n.size();
//    label_status->setText(tr("正在释放文件"));
//#ifdef Q_OS_WIN
//    timeEndPeriod(1);
//#endif
//    releaseFile(n);
//    label_status->setText(tr("文件释放成功"));
//    send("FILE_RELEASE_SUCCESSFULLY");
    
//    //裂变传播
//    if(!sendTask.empty()){
//        label_status->setText("开始传播文件");
//        process_events_without_useript;
//        for(auto t : sendTask){
//            //查找
//            int index = -1;
//            for(int i=0;i<clients.size();i++){
//                if(clients[i].operator QString const() == t){
//                    index=i;
//                    break;
//                }
//            }
////            QMetaObject::invokeMethod(this,"sendFileTo",Qt::QueuedConnection,Q_ARG(int,index));//QueuedConnection在事件循环运行并且顺序按照invoke的顺序运行。sendFileTo不能在除了事件循环以外的其他地方运行
//            sendFileTo(index);
//            ui->textEdit_debug1->append(QString("发送文件到%1").arg(index));
//        }
//        sendTask.clear();//清空以便下次
//    }
//}


//void MainWindow::on_settings_saved(){
////    bool flag = ;//用户名密码是否变更
//    bool restart_flag = false;
    
    
//    if(user_name!=ui->lineEdit_settings_username->text()||pwd!=ui->lineEdit_settings_pwd->text()){//保存用户名密码
//        auto un=ui->lineEdit_settings_username->text(),p=ui->lineEdit_settings_pwd->text();
//        if(un.size()<8||p.size()<8){
//            QMessageBox::warning(this,"无法设置用户名密码","用户名密码过短。要求用户名和密码不少于8字符");
//            return;
//        }
//        if(user_name!=ui->lineEdit_settings_username->text() && !QDir("config/empty/label1/ext-label-private/label-SyncTunnel-Username-uploaded/").exists()){
//            int btn = QMessageBox::information(this,"首次设置用户名防抢注验证","为同一用户防止恶意抢注、占用多个用户名、保障所有用户公平使用P2P功能，我们需要完成一次匿名验证：\n"" 仅上传您用户名的匿名加密串（攻击者不可能还原用户名明文，无任何个人信息）；\n"" 仅首次设置用户名时操作一次，后续不再上传任何数据；\n"" 数据存储于国内服务器，在验证用户名不是恶意抢注或占有后会尽快自动删除，数据最长留存一年（可联系nnpyro2@outlook.com删除）\n重要：若拒绝验证，您只能使用默认用户名+自定义密码使用软件，默认用户名为多用户共享，安全性低，若密码发生碰撞，您的个人数据可能会泄露，强烈不推荐使用。若使用，请设置强密码，并对自己的数据安全负全责\n\n""是否确认完成验证并保存用户名？",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
//            if(btn==QMessageBox::No){
//                QMessageBox::information(this,"用户名未验证","用户名未经过防抢注验证，未保存，自动使用默认用户名");
//                ui->lineEdit_settings_username->setText("DefaultUser");
//                if(p.size()<15){
//                    QMessageBox::warning(this,"强烈建议","您正在使用默认用户名，默认用户名安全性极低，我们强烈建议您的密码长度大于15字符！！！！！\n\n\n或者您可以直接设置一个专属用户名！使用默认用户名的安全性极低，极易发生密码碰撞/数据泄露的重大安全事故，如果您使用默认用户名，请对自己的数据安全负全责！");
////                    return;
//                }
//            }
//            else{
//                label_status->setText("正在进行用户名防抢注验证……");
//                auto manager = new QNetworkAccessManager(this);
//                QNetworkRequest request;
//                //构造请求
//                //            request.setRawHeader("Authorization","Bearer github_pa""t_11BF");
//                request.setHeader(QNetworkRequest::UserAgentHeader,"NNPYRO SyncTunnel Service");
//                //            request.setRawHeader("Accept", "application/vnd.github.v3+json");
//                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//                QJsonObject json;
                
//                json["message"]="用户名滥用检查";
////                json["access_token"]=SYNCTUNNEL_INTERFACE_W_ACCESS_TOKEN;
//                request.setRawHeader("PRIVATE-TOKEN",SYNCTUNNEL_INTERFACE_W_ACCESS_TOKEN);
//                json["branch"]="username";
//                //设置统计文件
//                request.setUrl(QUrl(QString("https://api.gitcode.com/api/v5/repos/2501_93498940/synctunnel-interface-w/contents/users/%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))));
//                json["content"]=QString(QString("Time:%1\nUser-Name:%2\n").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")).arg(QString(QCryptographicHash::hash(ui->lineEdit_settings_username->text().toUtf8(),QCryptographicHash::Sha256).toHex())).toUtf8().toBase64());
//                QNetworkReply *reply = manager->post(request,QJsonDocument(json).toJson());
//                //等待响应
//                QEventLoop loop;
//                connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
//                QTimer::singleShot(15000,&loop,&QEventLoop::quit);
//                loop.exec();
//                if(reply->error() != QNetworkReply::NoError || !reply->isFinished()){
//                    QMessageBox::critical(this,"错误","错误：无法请求数据到服务器。详细信息："+reply->errorString());
//                    //不允许用户暂时使用
//                    return;
//                }
//                else{
//                    QMessageBox::information(this,"成功","验证成功！您的用户名设置成功，应用程序自动重启。");
//                    QDir("config/empty/label1/ext-label-private/label-SyncTunnel-Username-uploaded/").mkpath(".");//下次不上传
//                }
//                reply->deleteLater();
//                manager->deleteLater();
//            }
//        }
//        user_name = ui->lineEdit_settings_username->text();
//        pwd = ui->lineEdit_settings_pwd->text();
//        QDir dir("config");
//        if(!dir.exists())dir.mkpath(".");
//        QFile f("config/1.nprivate0");
//        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
//        QByteArray iv;iv.resize(16);
//        QRandomGenerator::global()->generate(iv.data(),iv.data()+16);
//        auto buf = QString("%1\n%2").arg((QString)user_name.toUtf8().toBase64()).arg(/*(QString)pwd.toUtf8().toBase64()*/(QString)((iv+QAESEncryption::Crypt(QAESEncryption::AES_256,QAESEncryption::CBC,pwd.toUtf8(),SYNCTUNNEL_CRPT_KEY,iv)).toBase64())).toUtf8();
//        f.write(buf);//写入
//        f.close();
//        restart_flag=true;
//    }
//    if(ui->lineEdit_settings_mqttServer->text() != mqtt_server.ip || ui->spinBox_settings_mqttPort->value() != mqtt_server.port){
//        mqtt_server = ipport{ui->lineEdit_settings_mqttServer->text(),(quint16)ui->spinBox_settings_mqttPort->value()};
//        QDir dir("config");
//        if(!dir.exists())dir.mkpath(".");
//        QFile f("config/2.nprivate0");
//        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
//        f.write(QString("%1\n%2").arg(QString(mqtt_server.ip.toUtf8().toBase64())).arg(mqtt_server.port).toUtf8());
//        f.close();
//        restart_flag=true;
//    }
//    if(ui->lineEdit_settings_gitubUser->text() != user_github_name || ui->lineEdit_settings_githubPAT->text() != user_github_PAT){
//        user_github_name = ui->lineEdit_settings_gitubUser->text();
//        user_github_PAT = ui->lineEdit_settings_githubPAT->text();
//        QDir dir("config");
//        if(!dir.exists())dir.mkpath(".");
//        QFile f("config/3.nprivate0");
//        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
//        f.write(QString("%1\n%2").arg(QString(user_github_name.toUtf8().toBase64())).arg(QString(user_github_PAT.toUtf8().toBase64())).toUtf8());
//        f.close();
//        restart_flag=true;
//    }
//    if(ui->comboBox_settings_uiskin->currentData() != currentSkin){
//        currentSkin=(skinType)ui->comboBox_settings_uiskin->currentData().toInt();
//        QDir dir("config");
//        if(!dir.exists())dir.mkpath(".");
//        QFile f("config/4.nprivate0");
//        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
//        f.write(QString::number(currentSkin).toUtf8());
//        f.close();
//        restart_flag=true;
//    }
//    if(ui->checkBox_settings_recordLog->isChecked()!=json_settings["use_log"].toBool() || ui->checkBox_settings_disableNotice->isChecked()!=json_settings["disable_notice"].toBool() || ui->lineEdit_settings_description->text()!=device_description){
//        json_settings["use_log"]=ui->checkBox_settings_recordLog->isChecked();
//        json_settings["disable_notice"]=ui->checkBox_settings_disableNotice->isChecked();
//        json_settings["description"]=ui->lineEdit_settings_description->text();
//        QFile f("config/config.json");
//        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
//        f.write(QJsonDocument(json_settings).toJson());
//        f.close();
//        restart_flag=true;
//    }
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
    
    
//    if(restart_flag){//重启
//        m_signalling->exit();//发布关闭消息
//        QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());//重启
//        QCoreApplication::quit();
//    }
//}


//void MainWindow::on_hangup(){
//    label_status->setText(tr("正在挂起文件"));
//    QByteArray fileContent = encode(mergeFile(QDir("files/")));
//    if(m_storage->upload(fileContent)){
//        label_status->setText(tr("文件挂起成功"));
//        QMessageBox::information(this,"文件挂起",tr("文件挂起成功！"));
//    }
//    else{
//        label_status->setText(tr("文件挂起失败"));
//        QMessageBox::critical(this,"文件挂起",tr("文件挂起失败！\n\n可能是由于网络波动、服务器关闭等原因。\n文件不能重复挂起，如果第一次文件挂起成功那么第二次重复挂起必定失败。建议检查文件挂起状态是否为“已挂起”。\nGitHub限制每小时最多5000请求，请勿频繁操作文件。（解决办法参见帮助文档）\n\n详细信息参见帮助文档。"));
//    }
//}


//void MainWindow::on_download(){
//    label_status->setText("正在下载挂起的文件");
//    QByteArray fileContent = m_storage->get();
//    if(fileContent.isEmpty()){
//        label_status->setText("文件下载失败！");
//        QMessageBox::critical(this,"文件挂起","文件下载失败！");
//        return;
//    }
////    ndb<<"文件内容："<<decode(fileContent);
//    releaseFile(decode(fileContent));
//    label_status->setText("正在下载挂起的文件|即将完成|您现在已经可以使用下载的文件");
//    if(m_storage->remove()){
//        label_status->setText("文件下载成功");
//        QMessageBox::information(this,"文件挂起","文件下载成功");
//    }
//    else{
//        label_status->setText("文件下载失败");
//        QMessageBox::critical(this,"文件挂起","文件下载失败");
//    }
//}


//void MainWindow::on_proxy(){
//#ifdef Q_OS_WIN
//    if(ui->pushButton_switchProxy->text() != "开始加速" || ui->pushButton_switchProxy->text() != "停止加速") ui->pushButton_switchProxy->setText("开始加速");//规范化按钮文字
//    if(ui->pushButton_switchProxy->text() == "开始加速"){
//        //开启加速进程
//        process_proxy->setProcessChannelMode(QProcess::MergedChannels);
//        process_proxy->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args){args->flags=args->flags|CREATE_NO_WINDOW;});
///*        process_proxy->start("tools/fastgithub.exe");
//        process_proxy->waitForStarted();*/
//        //开启UI进程
////        process_proxy_ui->start("tools/FastGithub.UI.exe");
//        ui->pushButton_switchProxy->setEnabled(true);
//        ui->pushButton_switchProxy->setText("停止加速");
//        //开启加速进程
//        process_proxy->start("tools/fastgithub.exe");
//        process_proxy->waitForStarted();
////        ndb<<"process_proxy_ui->errorString()="<<process_proxy_ui->errorString();
//        if(process_proxy->state() == QProcess::Running){
//            label_status->setText("加速启动成功");
//        }
//        else{
//            label_status->setText("加速启动失败。状态："+QString::number(process_proxy->state())+process_proxy->errorString());
//            //启动管理员权限
//            HINSTANCE hs = ShellExecuteA(nullptr,"runas",QApplication::applicationFilePath().toStdString().c_str(),(QApplication::arguments()<<"-proxy").join(" ").toStdString().c_str(),nullptr,SW_SHOWNORMAL);
//            if(hs > (HINSTANCE)32){
//                ninfo<<"成功申请管理员权限";
//                process_proxy->terminate();
//                process_proxy->waitForFinished();
////                process_proxy_ui->terminate();
////                process_proxy_ui->waitForFinished();
//                close();
//            }
            
//        }
//    }
//    else if(ui->pushButton_switchProxy->text() == "停止加速"){
//        QProcess::startDetached("tools/fastgithub.exe",{"stop"});
//        QThread::msleep(1000);
//        QCoreApplication::processEvents();
////        process_proxy_ui->terminate();
////        process_proxy_ui->waitForFinished();
//        process_proxy->terminate();
//        process_proxy->waitForFinished(3000);
//        if(process_proxy->state()==QProcess::Running)process_proxy->kill();
//        ui->pushButton_switchProxy->setText("开始加速");
//        QCoreApplication::processEvents();
//        /*process_proxy->start("tools/fastgithub.exe",{"stop"});
//        process_proxy->waitForStarted();
//        process_proxy->terminate();*/
////        process_proxy_ui->terminate();
////        process_proxy_ui->waitForFinished();
//        QProcess::startDetached("ipconfig",{"/flushdns"});
//        ui->textBrowser_proxy->clear();
//    }
//#else
//    QMessageBox::warning(this,"警告","警告：只有Windows电脑才可以运行网络加速.");
//#endif
//}


//void MainWindow::on_rightclick_deviceList(){
//    QMenu *rightMenu = new QMenu;
//    rightMenu->setAttribute(Qt::WA_DeleteOnClose);
//    rightMenu->addAction(ui->actionShutdown_current);
//    rightMenu->addAction(ui->actionTest_RTT);
//    rightMenu->addAction(ui->actionRequestFile);
//    rightMenu->addAction(ui->actionRemoteCopyFile);
//    rightMenu->addAction(ui->actionSend_message);
//    rightMenu->addAction(ui->actionStart_remote);
//    rightMenu->exec(QCursor::pos());
//}


//void MainWindow::on_test_rtt(){
//    rtt_result.clear();
//    elapsed_rtt.start();
    
//    //发送测试信息
//    send("{\n    \"opt\":\"rtt_test\"\n}");
    
//    QEventLoop loop;
//    QTimer timer;
//    connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
//    timer.start(2000);
//    loop.exec();
    
//    //显示
//    elapsed_rtt.invalidate();
//    for(auto it=rtt_result.begin();it!=rtt_result.end();it++){
//        ui->tableWidget_deviceList->item(it.key(),3)->setText(QString("RTT=%1 delay=%2").arg(it.value()).arg(it.value()/2));
//    }
//}


//void MainWindow::on_SPTP_ctrlMsg_received(TransmissionEngine::msg_ctrl msg){
//    ninfo<<"收到控制消息 ctrl="<<msg.ctrl<<"value="<<msg.value;
    
//    if(msg.ctrl=="RESTART_NETWORK"){
//        restart();
//    }
//}


//void MainWindow::restart(){
//#if defined (Q_OS_WIN) || defined(Q_OS_LINUX)
//    QProcess::startDetached(qApp->applicationFilePath(),qApp->arguments());
//    close();
//    QApplication::processEvents(QEventLoop::AllEvents,1000);
//    multiDelay(3);
//    exit(0);
//#elif defined(Q_OS_ANDROID)
//    QAndroidJniObject activity = QtAndroid::androidActivity();
//    if (!activity.isValid()) return;

//    // 1. 获取包名
//    QAndroidJniObject packageName = activity.callObjectMethod(
//        "getPackageName", "()Ljava/lang/String;"
//    );

//    // 2. 系统核心：创建启动 Intent
//    QAndroidJniObject intent = QAndroidJniObject::callStaticObjectMethod(
//        "android/content/Intent",
//        "makeMainActivityIntent",
//        "(Ljava/lang/String;)Landroid/content/Intent;",
//        packageName.object()
//    );

//    // 3. 关键：清空任务栈，全新冷启动
//    intent.callObjectMethod(
//        "addFlags", "(I)Landroid/content/Intent;",
//        0x10000000 | 0x00008000 | 0x00004000
//    );

//    // 4. 系统级服务：AlarmManager （进程死了也会执行！）
//    QAndroidJniObject context = activity.callObjectMethod(
//        "getApplicationContext", "()Landroid/content/Context;"
//    );
//    QAndroidJniObject alarmManager = context.callObjectMethod(
//        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
//        QAndroidJniObject::fromString("alarm").object()
//    );

//    // 5. 创建延迟执行的 PendingIntent（100ms 后启动）
//    jlong triggerTime = QAndroidJniObject::callStaticMethod<jlong>(
//        "android/os/SystemClock", "elapsedRealtime", "()J"
//    ) + 100; // 延迟100毫秒，给系统足够时间接收指令

//    QAndroidJniObject pendingIntent = QAndroidJniObject::callStaticObjectMethod(
//        "android/app/PendingIntent",
//        "getActivity",
//        "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;",
//        context.object(), 0, intent.object(), 0x10000000
//    );

//    // 6. 系统设置闹钟：100ms后强制启动应用
//    alarmManager.callMethod<void>(
//        "setExact", "(JILandroid/app/PendingIntent;)V",
//        0, triggerTime, pendingIntent.object()
//    );


//    return;
//#endif
//}


//void MainWindow::on_pushButton_debug1_clicked(){
////    ui->textBrowser_debug1->setText(mergeFile(QDir("files/")));
////    send({"{\n    \"test\":0\n}"});
////    sendFile();
////    ndb<<mergeFile(QDir("files"));
////    releaseFile(ui->textEdit_debug1->toPlainText());
////    m_storage->upload(QString("123abc*#NEW?&===NEW12345NEW45678===============================================================================================================================================================1234567890-/QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1AQ2WS3ED4RF5TG6YH7UJ8IK9OL0QAZWSXEDCRFVTGBYHNUJMIK,OVTBNCMEX,O.BYCUNEX,1222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222 \nEND").toUtf8());
////    m_storage->remove();
////    ui->textBrowser_debug1->append(m_storage->get());
//    /*ndb << "模拟关机事件";
    
//    // 模拟WM_QUERYENDSESSION消息
//    MSG msg;
//    msg.hwnd = (HWND)winId();
//    msg.message = WM_QUERYENDSESSION;
//    msg.wParam = 0;
//    msg.lParam = 0;
//    msg.time = GetTickCount();
//    msg.pt = {0, 0};
    
//    long result = 0;
//    nativeEvent("windows_generic_MSG", &msg, &result);
//    ndb<<"var result ="<<result;*/
////    ndb<<checkSkin(Golden);
////    QApplication::clipboard()->setText(mergeFile(QDir("files/")));
//    /*/{
//        QFile f("logs/mergeFileData");
//        f.open(QIODevice::WriteOnly);
//        f.write(mergeFile(QDir("files/"),0));
//        f.close();
//        f.deleteLater();
//    }/*/
//    /*{
//        releaseFile("FILE\nAttack.dll\n5\n123123123");
//    }*/
////    sendFileTo(1);
//    /*planAutoSend({{"127.0.0.1",8080},
//                  {"114.114.114.114",12345},
//                  {"8.8.8.8",12345},
//                  {"234.123.12.1",1234},
//                  {"203.174.65.18", 45123},
//                  {"142.93.178.205", 31567},
//                  {"64.227.123.89", 28945},
//                  {"192.168.0.214", 54321},
//                  {"172.217.168.142", 40256},
//                  {"104.248.150.37", 37890},
//                  {"139.59.211.248", 45678},
//                  {"167.99.135.32", 32109},
//                  {"68.183.222.111", 49876},
//                  {"138.197.192.100", 31234}});/*/
//    /*try {
//        Dialog_selectSyncDst  *dialog = new Dialog_selectSyncDst(this);
//        dialog->setup({{"127.0.0.1",8080},
//                       {"114.114.114.114",12345},
//                       {"8.8.8.8",12345},
//                       {"234.123.12.1",1234},
//                       {"203.174.65.18", 45123},
//                       {"142.93.178.205", 31567},
//                       {"64.227.123.89", 28945},
//                       {"192.168.0.214", 54321},
//                       {"172.217.168.142", 40256},
//                       {"104.248.150.37", 37890},
//                       {"139.59.211.248", 45678},
//                       {"167.99.135.32", 32109},
//                       {"68.183.222.111", 49876},
//                       {"138.197.192.100", 31234}});
//        dialog->exec();
//    } catch (...) {
//        ncritical<<"nullptr";
//    }*/
    
////    dialog->show();
////    QEventLoop loop;
    
////    dialog->deleteLater();
////    sendReliableMessage(1,"这是三二可靠消息1-------2-------3-------4-------5-------6-------7-------8-------");
////    planAutoSend({
////                     {"1",1},
////                     {"2",1},
////                     {"3",1},
////                 });
////    QMetaObject::invokeMethod(this,"sendFileTo",Qt::QueuedConnection,Q_ARG(int,1));
////    QMetaObject::invokeMethod(this,"sendFileTo",Qt::QueuedConnection,Q_ARG(int,2));
////    m_transmissionengine->send("啊啊啊啊啊啊啊啊啊");
//    /*{
//        label_status->setText("正在发送可靠消息");
//        bool a = m_transmissionengine->sendReliableMessage(1,"DING");
//        label_status->setText(QString("发送可靠消息完成。成功：%1").arg(a));
//    }*/
////    QApplication::beep();
////    QSound::play(":/rc/audio/file_send_successfully.wav");
////    playSound(QUrl("qrc:/rc/audio/file_send_successfully.wav"));
////    ndb<<generateFileHashMap(syncFolder);
////    m_transmissionengine->SPTP_sendCtrl("RESTART_NETWORK","",-2);
//    ninfo<<traverseFolder(QDir("files/"));
//}



void MainWindow::closeEvent(QCloseEvent *event){
    //退出窗口
    ninfo<<"spontaneous "<<event->spontaneous();
    if(event->spontaneous()){//用户自主点击
        if((QGuiApplication::keyboardModifiers() & Qt::ControlModifier)!=0){//按下Ctrl
//            m_signalling->exit();//发布关闭消息
            vm->on_suspended();//发布关闭消息
            event->accept();
        }
        else{
            event->ignore();
            hide();
            return;
        }
    }
    else{//程序中的close
        vm->on_suspended();//发布关闭消息
        event->accept();
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    if(event->mimeData()->hasUrls()){
        event->acceptProposedAction();
        event->accept();
    }
}


void MainWindow::dropEvent(QDropEvent *event){
    QList<QUrl> list = event->mimeData()->urls();
    if(list.empty()){
        return;
    }
    foreach(auto i , list){
        QFileInfo fileInfo(i.toLocalFile());
        QString destPath = vm->o_current_dir.get().absoluteFilePath(fileInfo.fileName());
        QFile::copy(fileInfo.filePath(), destPath);
    }
    show_dir();
}


bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result){
#ifdef Q_OS_WIN
    MSG *msg = (MSG*)message;
    
//    if((msg->message == WM_QUERYENDSESSION||msg->message == WM_ENDSESSION)&&(!is_accept_shutdown)){//关机事件
//        *result=FALSE;
        
//        ndb<<"IN2";
//        QMetaObject::invokeMethod(this,[this]{
//            is_accept_shutdown = true;
//            long btn= QMessageBox::warning(this,"文件同步关机警告：请不要关机",tr("SyncTunnel检测到您正在关机\n\n如果您有文件同步需求（在此设备离线后仍然可以在其他设备上看到此设备上的文件），请不要关机，而是进入“低功耗模式”，让系统保持低功耗运行，文件同步后自动关机。\n\n如果您想进入上述“低功耗模式”，请点击“是”\n如果您想彻底关闭这台计算机，请点击“否”。\n\n您可以在“设置”页面配置关机时的行为。"),(QMessageBox::Yes|QMessageBox::No));
//            if(btn==QMessageBox::Yes){
//                int min = QInputDialog::getInt(this,"设置关机时间",tr("请输入关机时间 单位：分钟。\n关机时间指在您输入的分钟之后，即使没有文件同步请求，也直接关机。\n默认：5小时"),300,0,35791,60);
//                timer_savePower_finish.start(min * 60 * 1000);
//                savePower();
                
//            }
//            else if(btn==QMessageBox::No){
//                ShutdownBlockReasonDestroy((HWND)winId());
//                QProcess::startDetached("shutdown",{"-s","-t","10"});
//                close();
//            }
//        },Qt::QueuedConnection);
//        return true;
//    }
#endif
    
    return QWidget::nativeEvent(eventType,message,result);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event){
//    if (obj == widget_savePower && event->type() == QEvent::Close) {
//        // 处理省电模式窗口的关闭事件
//        if (widget_savePower) {
//            widget_savePower->hide();
//        }
//        this->show(); // 显示主窗口
//        timer_savePower_finish.stop(); // 停止关机倒计时
//        timer_savePower.stop();
        
//        // 解除关机阻止
//#ifdef Q_OS_WIN
//            ShutdownBlockReasonDestroy((HWND)winId());
//#endif
        
//        return true; // 事件已处理
//    }
//    if(obj == ui->label_remote_screen && ui->label_remote_screen->isEnabled()){
//        if(event->type()==QEvent::MouseButtonPress){
//            send(QString("{\n    \"remote_event\":\"mouse_move\",\n    \"mx\":%1,\n    \"my\":%2\n}").arg(static_cast<QMouseEvent*>(event)->x()*1./ui->label_remote_screen->width()).arg(static_cast<QMouseEvent*>(event)->y()*1./ui->label_remote_screen->height()).toUtf8(),1,clients.indexOf(remote_device));
//            return true;
//        }
//    }
    if(event->type()==QEvent::Wheel && (obj->objectName().contains("comboBox")||obj->objectName().contains("spinBox"))){
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}



void MainWindow::resizeEvent(QResizeEvent *e){
    androidRun{
            int width = ui->tabWidget->size().width()/4;
//            ndb<<"width"<<width;
//            ninfo<<width;
            ui->tabWidget->tabBar()->setStyleSheet(/*ui->tabWidget->tabBar()->styleSheet()+*/QString("QTabBar::tab:enabled{ width: %1px; margin: 0px; padding: 0px; }").arg(width));
            ui->tabWidget->tabBar()->setExpanding(true);
            ui->tabWidget->setTabPosition(QTabWidget::South);
            hideTab(ui->tabWidget,2);
            hideTab(ui->tabWidget,3);
            hideTab(ui->tabWidget,5);
            hideTab(ui->tabWidget,6);
    };
}


