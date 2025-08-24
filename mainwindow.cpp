#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfoList>
#include <QDesktopServices>
#include <QProcess>
#include <QFile>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../../libary/Qt-AES/qaesencryption.h"
#include <QCryptographicHash>
#include <sstream>
#include <QStack>
#include <QThread>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QFileIconProvider>

#define ndb qDebug()<<__FILE__<<__func__<<__LINE__<<"\n"
using namespace std;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //对象创建
    m_communication = new Communication;
    m_signalling = new Signalling;
    m_storage = new Storage;
    label_status = new QLabel(this);
    
    
    //基本设置
    ui->setupUi(this);
    resize(1200,900);
    setWindowTitle("SyncTunnel 同步隧道");
    ui->tabWidget->setTabEnabled(4,false);//禁用调试页面   //发布设置
    ui->tabWidget->setCurrentIndex(2);
    timer_is_uploading.setSingleShot(true);
    timer_clear_currentFileMap.setSingleShot(true);
    ui->tableWidget_deviceList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置自动列宽
//    ui->tableWidget_deviceList.
    //读取文件中的用户名密码
    QFile file1("config/1.nprivate0");
    file1.open(QIODevice::ReadOnly);
    QTextStream user_config_stream1(&file1);
    user_name = QByteArray::fromBase64(user_config_stream1.readLine().toUtf8());
    pwd = QByteArray::fromBase64(user_config_stream1.readLine().toUtf8());
    ui->lineEdit_settings_username->setText(user_name);
    ui->lineEdit_settings_pwd->setText(pwd);
    ui->statusBar->addPermanentWidget(label_status);
    //读取文件中的mqtt服务器设置
    QFile file2("config/2.nprivate0");
    file2.open(QIODevice::ReadOnly);
    QTextStream user_config_stream2(&file2);
    mqtt_server = ipport{
            QString(QByteArray::fromBase64(user_config_stream2.readLine().toUtf8())),
            static_cast<quint16>(user_config_stream2.readLine().toInt()^qHash(pwd))
    };
    ui->lineEdit_settings_mqttServer->setText(mqtt_server.ip);
    ui->spinBox_settings_mqttPort->setValue(mqtt_server.port);
    //读取文件中的Github服务器设置
    QFile file3("config/3.nprivate0");
    file3.open(QIODevice::ReadOnly);
    QTextStream user_config_stream3(&file3);
    user_github_name = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
    user_github_PAT = QByteArray::fromBase64(user_config_stream3.readLine().toUtf8());
    ui->lineEdit_settings_gitubUser->setText(user_github_name);
    ui->lineEdit_settings_githubPAT->setText(user_github_PAT);
    //设置当前页
    QDir dir_empty_label1("config/empty/label1");
    if(dir_empty_label1.exists()){
        ui->tabWidget->setCurrentIndex(0);
    }
    else{
        dir_empty_label1.mkpath(".");
    }
    
    
    //信号槽绑定
    connect(ui->listWidget_file,&QListWidget::doubleClicked,this,&MainWindow::on_folder_change);
    connect(ui->listWidget_file,&QListWidget::customContextMenuRequested,this,&MainWindow::on_rightclick);
    connect(ui->actionOpen,&QAction::triggered,this,[this]{
//        ndb<<QStringList(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath());
        QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators(QDir(current_dir.filePath(ui->listWidget_file->currentItem()->text())).absolutePath())));
    });
    connect(ui->actionFolder,&QAction::triggered,this,[this]{QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators((current_dir).absolutePath())));});
    connect(m_communication,&Communication::readyRead,this,&MainWindow::on_readyRead);
//    if(1)connect(m_signalling,&Signalling::on_userlist_updata,this,[this](QList<Communication::ipport> userl){qDebug()<<"信号触发";ui->textBrowser_debug1->clear();foreach(auto i,userl)ui->textBrowser_debug1->append(i);});
    connect(m_signalling,&Signalling::on_userlist_updata,this,[this](QList<Communication::ipport> userl){
        clients = userl;if(1)foreach(auto i,clients)qDebug()<<i;
        send("{\n    \"hole\":1\n}");
        label_status->setText("用户列表更新成功");
        ui->tableWidget_deviceList->clearContents();ui->tableWidget_deviceList->setRowCount(0);ndb<<"IN";
        
        foreach(auto i,clients){        //表格设置
            int r=ui->tableWidget_deviceList->rowCount();
            ui->tableWidget_deviceList->insertRow(r);
            ui->tableWidget_deviceList->setItem(r,0,new QTableWidgetItem(user_name));
            ui->tableWidget_deviceList->setItem(r,1,new QTableWidgetItem(i.ip));
            ui->tableWidget_deviceList->setItem(r,2,new QTableWidgetItem(QString::number(i.port)));
            ui->tableWidget_deviceList->setItem(r,3,new QTableWidgetItem("暂时不支持备注功能"));
        }
        if(1){ui->textBrowser_debug1->clear();ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));foreach(auto i,clients)ui->textBrowser_debug1->append(i);}
    });
    connect(&timer_fileResend,&QTimer::timeout,this,&MainWindow::on_request_resend); 
    connect(&timer_is_uploading,&QTimer::timeout,this,[this]{chunks.clear();is_uploading=false;});
    connect(ui->pushButton_settings_save,&QPushButton::clicked,this,&MainWindow::on_settings_saved);
    connect(ui->actionupload_file,&QAction::triggered,this,[this]{sendFile();});
    connect(&timer_clear_currentFileMap,&QTimer::timeout,this,[this]{currentFileMap.clear();currentFileTotal = -1;});
    connect(ui->actionHangup,&QAction::triggered,this,&MainWindow::on_hangup);
    connect(ui->actionDownload,&QAction::triggered,this,&MainWindow::on_download);
    connect(ui->actionSync_PAT,&QAction::triggered,this,[this]{
        QJsonObject json;json.insert("pat",ui->lineEdit_settings_githubPAT->text());
        send(QJsonDocument(json).toJson());
    });
    
    
    //目录显示
    current_dir = QDir("files/");
    show_dir();
    
    
    //网络部分
    //发起STUN
    show();
    label_status->setText("正在获取公网IP……");
    QCoreApplication::processEvents();
    public_ip = m_communication->stun();
    if(public_ip == Communication::ipport{"",0} ||0/*调试 强制IPV6*/){
        QCoreApplication::processEvents();
        
        //开始用IPv6
        public_ip = m_communication->getIPv6();
        
        if(public_ip == Communication::ipport{"",0}){
            QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());
            QCoreApplication::quit();
            abort();
        }
    }
    label_status->setText("正在获取上线用户列表……");
    ndb<<"var:public ip="<<public_ip;
    //MQTT
    m_signalling->connectToHost(/*{"broker.emqx.io",1883}*/mqtt_server);
    m_signalling->subscribe("nnpyro_syncTunnel/user_topics/" + user_name);
    m_signalling->setPwd(pwd.toUtf8());
    m_signalling->setUser(public_ip,user_name);
    clients = m_signalling->getUserList();//获取用户列表
    if(1)foreach(auto i,clients)qDebug()<<i;
    label_status->setText("加载成功");
    //打洞
//    send("{\n    \"hole\":1\n}");
    
    //等待直到用户列表获取完成
    QEventLoop el1;
    connect(m_signalling,&Signalling::on_userlist_updata,&el1,&QEventLoop::quit);
//    el1.exec(QEventLoop::ExcludeUserInputEvents);
//    QCoreApplication::processEvents();
    
    //调试
    if(1)ui->textBrowser_debug1->append(QString("本机IP = %1").arg(public_ip));
    if(1)foreach(auto i,clients)ui->textBrowser_debug1->append(i);
    
    //设置文件挂起
    m_storage->setUser(user_github_name,user_github_PAT);
    
}

MainWindow::~MainWindow(){
    m_communication->deleteLater();
    m_signalling->deleteLater();
    delete ui;
}


void MainWindow::show_dir(){        //显示目录
    ui->listWidget_file->clear();
    current_dir.refresh();
//    ndb<<"var:current_dir"<<current_dir.absolutePath()<<"   exists"<<current_dir.exists();
    QFileInfoList fil = current_dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsFirst);
//    ndb<<"list size:"<<fil.size();
    
    //添加上级目录
    if(current_dir != QDir("files")){//不能更往前了
//        ndb<<"PRE";
        QListWidgetItem *item = new QListWidgetItem;
        item->setText("(上级目录)");
        item->setData(Qt::UserRole,QVariant("pre"));//设置标签
        ui->listWidget_file->addItem(item);
    }

    
    //遍历并输出
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
            ui->listWidget_file->addItem(item);
        }
    }
}


void MainWindow::send(QByteArray msg, bool e){
    //自动补全信息
    QJsonDocument jd = QJsonDocument::fromJson(msg);
    if(jd.isObject()){
        QJsonObject json = jd.object();
        if(!json.contains("user")){
            json.insert("user",user_name);
        }
        msg = QJsonDocument(json).toJson();
    }
    
    //加密并发送
    foreach(auto i,clients)m_communication->send(i,e?encode(msg):msg);
}


QByteArray MainWindow::encode(QByteArray msg){
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
    QByteArray encode = encription.encode(msg,key,iv)/*.toBase64()*/;
    return encode;
}


QByteArray MainWindow::decode(QByteArray msg){
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
    QByteArray decoded = encription.removePadding(encription.decode(/*QByteArray::fromBase64(msg)*/msg,key,iv));
    return decoded;
}


/*QString MainWindow::mergeFile(QDir folder){
    QString f;
    QFileInfoList info = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsLast);
    ndb<<"进入目录"<<folder.absolutePath()<<"长度"<<info.size();
    foreach(QFileInfo fi ,info){
        if(fi.isFile()){
            ndb<<"处理文件"<<fi.absoluteFilePath();
            f += "FILE\n";
//            QString filepath = fi.canonicalFilePath();
//            QString dirpath = QDir("files/").canonicalPath();
//            f += filepath.mid(dirpath.size()+1) + "\n";
            QString absPath = fi.canonicalFilePath();
            QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
            f += relativePath + "\n";
            QFile file(absPath);
            file.open(QIODevice::ReadOnly);
            f += file.readAll().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals)/* + "\nEND_FILE\n"*/;
/*
        }
        else{//目录递归DFS
            ndb<<"处理目录"<<fi.absoluteFilePath();
            f += "DIR\n";
            QString absPath = fi.canonicalFilePath();
            QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
            f += relativePath + "\n";
//            f += mergeFile(absPath) + "END_DIR\n";
        }
    }
    return f;
}//*/
QString MainWindow::mergeFile(QDir folder){
    QString f;
    QFileInfoList info = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsLast);
    ndb<<"进入目录"<<folder.absolutePath()<<"长度"<<info.size();
    foreach(QFileInfo fi ,info){
        if(fi.isFile()){
            ndb<<"处理文件"<<fi.absoluteFilePath();
            f += "FILE\n";
//            QString filepath = fi.canonicalFilePath();
//            QString dirpath = QDir("files/").canonicalPath();
//            f += filepath.mid(dirpath.size()+1) + "\n";
            QString absPath = fi.canonicalFilePath();
            QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
            f += relativePath + "\n";
            QFile file(absPath);
            file.open(QIODevice::ReadOnly);
            f += qCompress(file.readAll(),9).toBase64() + "\n";
        }
        else{//目录递归DFS
            ndb<<"处理目录"<<fi.absoluteFilePath();
            f += "DIR\n";
            QString absPath = fi.canonicalFilePath();
            QString relativePath = QDir("files/").relativeFilePath(fi.filePath());
            f += relativePath + "\n";
//            f += mergeFile(absPath);
            f += mergeFile(QDir(relativePath));
        }
    }
    return f;
}



void MainWindow::sendFile(){
    //生成文件表
    is_uploading = true;
    label_status->setText("正在加载文件……");
    QString fileList = mergeFile((QDir)"files/");
    chunks.clear();
    
    //文件分片
    const int SPC = 3 * 1024;//3kb /*1400;//基于MTU1500的值*/
    label_status->setText("正在分片文件……");
    QTextStream stm(&fileList);
    for(;!stm.atEnd();){
        chunks.append(stm.read(SPC));
    }
    
    
    QJsonObject json;
    QList<QByteArray> send_buf;//发送缓冲区
//    json.insert("user",this->user_name);
    json.insert("total",chunks.size()-1);
    json.insert("filebody","");
    json.insert("no",-1);
    label_status->setText("正在加密文件……");
    for(int i=0;i<chunks.size();i++){//生成发送内容
        json["filebody"] = chunks[i];
        json["no"] = i;
        send_buf.push_back(encode(QJsonDocument(json).toJson()));
    }
    
    
    //分块发送
//    int loop_count = 0;//循环次数
    /*const int DELAY_LOOP = 1;
    for(int i=0;i<chunks.size();i++){
        json["filebody"] = chunks[i];
        json["no"] = i;
        QEventLoop loop;
        if(i % DELAY_LOOP == 0)QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
        send(QJsonDocument(json).toJson());
        if(i % DELAY_LOOP == 0)loop.exec(QEventLoop::ExcludeUserInputEvents);//循环两次停止一次
        else for(int j=0;j<100;j++);//挨时间
    }//*/
    
    //发送文件
    QElapsedTimer clock;
    clock.start();
    
    const int DELAY_LOOP = 1;
    for(int i=0;i<send_buf.size();i++){
        QEventLoop loop;
        if(i % DELAY_LOOP == 0)QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
        send(send_buf[i],0);
        if(i % DELAY_LOOP == 0)loop.exec(QEventLoop::ExcludeUserInputEvents);//循环两次停止一次
        else for(int j=0;j<100;j++);//挨时间
        if(i%3==0)label_status->setText(QString("正在发送文件：包%1/%2 时间：%3秒 速度%4b/s(%5kb/s)").arg(i).arg(send_buf.size()-1).arg(clock.elapsed()/1000).arg((i * SPC/(clock.elapsed()/1000.0)),0,'f',2).arg((i * SPC/(clock.elapsed()/1000.)/1024),0,'f',2));
//        QThread::usleep(5);
//        if(i % DELAY_LOOP == 0)QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    
//    QTimer *timer = new QTimer(this);
//    connect(timer,&QTimer::timeout,this,[this,&timer]{is_uploading = false;timer->deleteLater();});
//    timer->start(2000);
    label_status->setText("发送文件完毕");
    timer_is_uploading.start(10000);
}


void MainWindow::releaseFile(QString msg){
    QTextStream stm(&msg);
//    QStack<QDir> stack;
//    stack.push(QDir("files/"));
    
    while(!stm.atEnd()){//循环读取
        QString operation = stm.readLine();
        
        //解析
        if(operation == "FILE"){//解析文件
            QString filename = stm.readLine();//获取文件名
            QFile file(/*stack.top().filePath(filename)*/filename);
            file.open(QIODevice::WriteOnly);
            if(!file.isOpen()){
                ndb<<"Error:File isn't open.Details:"<<file.errorString();
            }
            
            QByteArray value = qUncompress(QByteArray::fromBase64(stm.readLine().toUtf8()));//文件内容
            file.write(value);
            file.close();
//            stm.readLine();
        }
        
        if(operation == "DIR"){//处理目录
            /*QString dirName = stm.readLine();
            
            QDir currentDir = stack.top();
            if(!QDir(currentDir.filePath(dirName)).exists()){
                currentDir.mkpath(dirName);
            }
            
//            stack.push(QDir(QDir(stack.top()).filePath(dirName)));*/
            
            QDir dir = stm.readLine();
            dir.mkpath(".");
        }
        
//        if(operation == "END_DIR"){//处理目录
//            if(!stack.empty()){
//                stack.pop();
//            }
//        }
    }
    
    //刷新目录
    show_dir();
}


void MainWindow::on_folder_change(){
    QListWidgetItem currentItem = *ui->listWidget_file->currentItem();
    
    if(currentItem.data(Qt::UserRole) == "folder"){//如果是目录就进入
        current_dir = QDir(current_dir.filePath(currentItem.text()));
        show_dir();//刷新列表
    }
    if(currentItem.data(Qt::UserRole) == "pre"){//上级目录
        current_dir.cdUp();
        show_dir();//刷新列表
    }
    if(currentItem.data(Qt::UserRole) == "file"){//是文件就打开
//        ndb<<QFile(QDir(current_dir.filePath(currentItem.text())).absolutePath()).exists()<<"    "<<QDir(current_dir.filePath(currentItem.text())).absolutePath();
        QProcess::startDetached("explorer.exe",QStringList(QDir::toNativeSeparators(QDir(current_dir.filePath(currentItem.text())).absolutePath())));
    }
}


void MainWindow::on_rightclick(){       //右键点击事件
//    ndb<<"click";
//    ndb<<mapFromGlobal(QCursor::pos());
    QMenu *rightMenu = new QMenu(this);
    if(ui->listWidget_file->itemAt(ui->listWidget_file->mapFromGlobal(QCursor::pos())) != NULL){//有Item的时候再菜单"打开"
//        ndb<<"ITEM";
        rightMenu->addAction(ui->actionOpen);
        
        if(ui->listWidget_file->currentItem()->data(Qt::UserRole) != "folder"){//目录不需要
            rightMenu->addAction(ui->actionFolder);
        }
    }
    else{
        rightMenu->addAction(ui->actionFolder);
    }
    
    rightMenu->setAttribute(Qt::WA_DeleteOnClose);
    rightMenu->exec(QCursor::pos());
}


void MainWindow::on_readyRead(){
    QNetworkDatagram datagram = m_communication->readDatagram();
    ipport sender = {datagram.senderAddress().toString(),(quint16)datagram.senderPort()};
//    ndb<<"data"<<datagram.data();
    //解密数据
    QByteArray msg = decode(datagram.data());
//    ndb<<"var:msg"<<msg;
    QJsonObject json_temp = QJsonDocument::fromJson(msg).object();json_temp.remove("filebody");
    ndb<<"var:msg(no filebody)"<<QJsonDocument(json_temp).toJson();
    
    QJsonDocument jd = QJsonDocument::fromJson(msg);
    QJsonObject json;
    
    if(!jd.isObject()){
        ndb<<"ERROR:msg isn`t an object!";
        return;
    }
    else{
        json = jd.object();
    }
    
    //消息解析
    if(json.contains("hole")){          //打洞
        QJsonObject replyJson;
        if(json["hole"].toInt() == 1){
            replyJson.insert("hole2",2);
//            send(QJsonDocument(json).toJson());
        }
        /*if(json["hole"].toInt() == 2){
            replyJson.insert("hole",3);
        }
        if(json["hole"].toInt() == 3     &&0){
            replyJson.insert("hole",4);
        }*/
    }
    if(json.contains("filebody")){
        if(currentFileTotal!=json["total"].toInt())currentFileMap.clear();
        currentFileTotal = json["total"].toInt();
        currentFileMap[json["no"].toInt()] = json["filebody"].toString();
        timer_fileResend.stop();
        timer_fileResend.start(3000);
        if(this->chunks.empty()){
            label_status->setText(QString("正在接收：包%1/%2").arg(json["no"].toInt()).arg(json["total"].toInt()));
        }
        if(currentFileMap.size() == currentFileTotal){
            label_status->setText("文件操作成功");
        }
    }
    if(json.contains("request_resend")/*&&is_uploading*/ && !chunks.empty()){
        QJsonObject rpjson;
    //    json.insert("user",this->user_name);
        rpjson.insert("total",chunks.size()-1);
        rpjson.insert("filebody","");
        rpjson.insert("no",-1);
            rpjson["filebody"] = chunks[json["request_resend"].toInt()];
            rpjson["no"] = json["request_resend"].toInt();
            send(QJsonDocument(rpjson).toJson());
            timer_is_uploading.stop();
            timer_is_uploading.start(10000);
            label_status->setText("正在回复重传请求……");
    }
    if(json.contains("pat")){
        ui->lineEdit_settings_githubPAT->setText(json["pat"].toString());
        ui->pushButton_settings_save->click();
    }
}


void MainWindow::on_request_resend(){
    //检测map连续 这个检测连续算法已经测试，没有问题
    auto it = currentFileMap.constBegin();
    int pre = -1;
//    it++;
    QList<int> resendList;
    for(;it != currentFileMap.constEnd();it++){
        if(it.key() != pre+1){
            for(int i=pre+1;i<it.key();i++){
                resendList.append(i);
            }
        }
        pre=it.key();
    }
    if(currentFileMap.lastKey() != currentFileTotal){
        for(int i=currentFileMap.lastKey()+1;i<=currentFileTotal;i++){
            resendList.append(i);
        }
    }
    
    ndb<<"var:list(丢包) ="<<resendList;
    
    
    //请求重传
    QJsonObject json;
    json.insert("request_resend",0);
    foreach(auto i , resendList){
        json["request_resend"] = i;
        send(QJsonDocument(json).toJson());
        label_status->setText(QString("正在请求重传%1个包").arg(resendList.size()));
        QThread::msleep(20);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents,100);
    }
    
    timer_fileResend.stop();
    if(resendList.size() != 0){//如果重新传输了那么就不能执行后面的合并文件否则会合并一个错误的文件
        return;
    }
    
    timer_clear_currentFileMap.stop();
    timer_clear_currentFileMap.start(10000);
    
    QEventLoop loop;        //定义一个新的事件循环
    QTimer::singleShot(500, &loop,&QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec(); 
    
    //合并文件
    QString n;
    foreach(QString i,currentFileMap){
        n.append(i);
    }
//    ui->textBrowser_debug1->setText(n);
    ndb<<"Log:文件请求已完成，字节数："<<n.size();
    label_status->setText("正在释放文件");
    releaseFile(n);
    label_status->setText("文件释放成功");
}


void MainWindow::on_settings_saved(){
//    bool flag = ;//用户名密码是否变更
    bool restart_flag = false;
    
    
    if(user_name!=ui->lineEdit_settings_username->text()||pwd!=ui->lineEdit_settings_pwd->text()){//保存用户名密码
        user_name = ui->lineEdit_settings_username->text();
        pwd = ui->lineEdit_settings_pwd->text();
        
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/1.nprivate0");
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(QString("%1\n%2").arg((QString)user_name.toUtf8().toBase64()).arg((QString)pwd.toUtf8().toBase64()).toUtf8());//写入
        f.close();
        restart_flag=true;
    }
    if(ui->lineEdit_settings_mqttServer->text() != mqtt_server.ip || ui->spinBox_settings_mqttPort->value() != mqtt_server.port){
        mqtt_server = ipport{ui->lineEdit_settings_mqttServer->text(),(quint16)ui->spinBox_settings_mqttPort->value()};
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/2.nprivate0");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QString("%1\n%2").arg(QString(mqtt_server.ip.toUtf8().toBase64())).arg(mqtt_server.port^(qHash(pwd))).toUtf8());
        f.close();
        restart_flag=true;
    }
    if(ui->lineEdit_settings_gitubUser->text() != user_github_name || ui->lineEdit_settings_githubPAT->text() != user_github_PAT){
        user_github_name = ui->lineEdit_settings_gitubUser->text();
        user_github_PAT = ui->lineEdit_settings_githubPAT->text();
        QDir dir("config");
        if(!dir.exists())dir.mkpath(".");
        QFile f("config/3.nprivate0");
        f.open(QIODevice::WriteOnly|QIODevice::Truncate);
        f.write(QString("%1\n%2").arg(QString(user_github_name.toUtf8().toBase64())).arg(QString(user_github_PAT.toUtf8().toBase64())).toUtf8());
        f.close();
        restart_flag=true;
    }
    
    
    if(restart_flag){//重启
        m_signalling->exit();//发布关闭消息
        QProcess::startDetached(QCoreApplication::applicationFilePath(),QCoreApplication::arguments());//重启
        QCoreApplication::quit();
    }
}


void MainWindow::on_hangup(){
    label_status->setText("正在挂起文件");
    QByteArray fileContent = encode(mergeFile(QDir("files/")).toUtf8());
    if(m_storage->upload(fileContent)){
        label_status->setText("文件挂起成功");
        QMessageBox::information(this,"文件挂起","文件挂起成功！");
    }
    else{
        label_status->setText("文件挂起失败");
        QMessageBox::critical(this,"文件挂起","文件挂起失败！\n\n可能是由于网络波动、服务器关闭等原因。\n文件不能重复挂起，如果第一次文件挂起成功那么第二次重复挂起必定失败。建议检查文件挂起状态是否为“已挂起”。\nGitHub限制每小时最多5000请求，请勿频繁操作文件。（解决办法参见帮助文档）\n\n详细信息参见帮助文档。");
    }
}


void MainWindow::on_download(){
    label_status->setText("正在下载挂起的文件");
    QByteArray fileContent = m_storage->get();
    if(fileContent.isEmpty()){
        label_status->setText("文件下载失败！");
        QMessageBox::critical(this,"文件挂起","文件下载失败！");
        return;
    }
    releaseFile(decode(fileContent));
    label_status->setText("正在下载挂起的文件|即将完成|您现在已经可以使用下载的文件");
    if(m_storage->remove()){
        label_status->setText("文件下载成功");
        QMessageBox::information(this,"文件挂起","文件下载成功");
    }
    else{
        label_status->setText("文件下载失败");
        QMessageBox::critical(this,"文件挂起","文件下载失败");
    }
}


void MainWindow::on_pushButton_debug1_clicked(){
//    ui->textBrowser_debug1->setText(mergeFile(QDir("files/")));
//    send({"{\n    \"test\":0\n}"});
//    sendFile();
//    ndb<<mergeFile(QDir("files"));
//    releaseFile(ui->textEdit_debug1->toPlainText());
//    m_storage->upload(QString("123abc*#NEW?&===NEW12345NEW45678===============================================================================================================================================================1234567890-/QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1AQ2WS3ED4RF5TG6YH7UJ8IK9OL0QAZWSXEDCRFVTGBYHNUJMIK,OVTBNCMEX,O.BYCUNEX,1222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222 \nEND").toUtf8());
//    m_storage->remove();
    ui->textBrowser_debug1->append(m_storage->get());
}



void MainWindow::closeEvent(QCloseEvent *event){
    //退出窗口
    m_signalling->exit();//发布关闭消息
    event->accept();
}


