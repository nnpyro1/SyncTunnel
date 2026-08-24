#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(ViewModel *_vm, QWidget *parent, std::function<void (QString)> msgLogger)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , vm(_vm) {
    ui->setupUi(this);
    Q_UNUSED(msgLogger);
    
    //设置标签页可见性
    auto tabBar = ui->tabWidget->tabBar();
    tabBar->setTabVisible(2,false);
    tabBar->setTabVisible(3,false);
    tabBar->setTabVisible(5,false);
    tabBar->setTabVisible(6,false);
    
    //Observable和信号槽
    // QAction *actionOpen;
    connect(ui->actionOpen, &QAction::triggered, vm, [this](){
        auto selRows = ui->listView_file->selectionModel()->selectedRows();
        if (selRows.isEmpty())
            return;
        int row = selRows.first().row();
        vm->openFile(row);
    });
    // QAction *actionFolder;
    connect(ui->actionFolder,&QAction::triggered,vm,&ViewModel::openFolder);
    // QAction *actionupload_file;
    connect(ui->actionupload_file,&QAction::triggered,vm,[this]{vm->sendFile();});
    // QAction *actionHangup;
    connect(ui->actionHangup,&QAction::triggered,vm,[this]{vm->on_hangup();});
    // QAction *actionDownload;
    connect(ui->actionHangup,&QAction::triggered,vm,&ViewModel::on_download);
    // QAction *actionSync_PAT;
    // connect(ui->actionSync_PAT,&QAction::triggered,vm,&ViewModel::on_download);
    // QAction *actionRefresh;
    connect(ui->actionRefresh,&QAction::triggered,vm,&ViewModel::refresh);
    // QAction *actionShutdown_current;
    connect(ui->actionShutdown_current,&QAction::triggered,vm,&ViewModel::on_shutdown_current);
    // QAction *actionShutdown;
    connect(ui->actionShutdown,&QAction::triggered,vm,&ViewModel::shutdown);
    // QAction *actionTest_RTT;
    connect(ui->actionTest_RTT,&QAction::triggered,vm,&ViewModel::on_test_rtt);
    // QAction *actionRequestFile;
    connect(ui->actionRequestFile,&QAction::triggered,vm,&ViewModel::on_request_file);
    // QAction *actionRemoteCopyFile;
    connect(ui->actionRemoteCopyFile,&QAction::triggered,vm,&ViewModel::startCopyingRemoteFile);
    // QAction *actionSend_message;
    connect(ui->actionSend_message,&QAction::triggered,vm,&ViewModel::sendTestMessage);
    // QAction *actionStart_remote;
    connect(ui->actionStart_remote,&QAction::triggered,vm,&ViewModel::on_start_remote);
    // QAction *actionOpen_DriveCrypto;
    connect(ui->actionOpen_DriveCrypto,&QAction::triggered,vm,&ViewModel::on_start_remote);
    // QAction *actionAdd_file;
    connect(ui->actionAdd_file,&QAction::triggered,vm,&ViewModel::addFile);
    // QAction *actionIncremental_sync_enable;
    connect(ui->actionIncremental_sync_enable,&QAction::triggered,vm,&ViewModel::addIncremental);
    // QAction *actionIncremental_sync_disable;
    connect(ui->actionIncremental_sync_disable,&QAction::triggered,vm,&ViewModel::removeIncremental);
    // QAction *actionExit_Application;
    connect(ui->actionExit_Application,&QAction::triggered,vm,&ViewModel::exit);
    // QAction *actionHang_up_file_to_dfhn;
    connect(ui->actionHang_up_file_to_dfhn,&QAction::triggered,vm,&ViewModel::on_hangup_to_dfhn);
    // QAction *actionDownload_file_from_dfhn;
    connect(ui->actionDownload_file_from_dfhn,&QAction::triggered,vm,&ViewModel::on_download_from_dfhn);
    // QAction *actionRestart_all_applications;
    connect(ui->actionRestart_all_applications,&QAction::triggered,vm,&ViewModel::on_restart_all);
    // QAction *actionrestart;
    connect(ui->actionrestart,&QAction::triggered,vm,&ViewModel::restart);
    // QWidget *centralwidget;
    // QGridLayout *gridLayout;
    // QTabWidget *tabWidget;
    vm->o_currentPageIndex.use(ui->tabWidget,[this]{ui->tabWidget->setCurrentIndex(vm->o_currentPageIndex);});
    connect(ui->tabWidget,&QTabWidget::currentChanged,[this](int i){vm->o_currentPageIndex=i;});
    // QWidget *tab_file;
    // QGridLayout *gridLayout_2;
    // QPushButton *pushButton_file_addSchedule;
    connect(ui->pushButton_file_addSchedule,&QPushButton::clicked,vm,&ViewModel::on_add_schedule);
    // QLabel *label_19;
    // QListWidget *listWidget_schedule;
    // ########## 预留！！！！ ##########   !!!!!!!!!!!!!!!!!!!!!!!!!
    // QPushButton *pushButton_file_syncAll;
    connect(ui->pushButton_file_syncAll,&QPushButton::clicked,vm,&ViewModel::clearIncremental);
    // QLabel *label_17;
    // QPushButton *pushButton_file_deleteCurrentSchedule;
    // ########## 预留！！！！ ##########   !!!!!!!!!!!!!!!!!!!!!!!!!
    // QCheckBox *checkBox_file_autoSync;
    // ########## 预留！！！！ ##########   !!!!!!!!!!!!!!!!!!!!!!!!!    
    // QListWidget *listWidget_file;
    ui->listView_file->setModel(&vm->fileModel);
    connect(ui->listView_file,&QListView::doubleClicked,vm,[this](QModelIndex &i){if(i.isValid()){vm->openFile(i.row());}});
    // QListWidget *listWidget_incremental;
    ui->listView_incremental->setModel(&vm->incrementalModel);
    // QListWidget *listWidget_schedule;
    // ########## 预留！！！！ ##########   !!!!!!!!!!!!!!!!!!!!!!!!!    
    // QWidget *tab_deviceList;
    // QGridLayout *gridLayout_7;
    // QGridLayout *gridLayout_12;
    // QFormLayout *formLayout;
    // QLabel *label_24;
    // QLabel *label_sendInfo_currentRate;
    // QLabel *label_25;
    // QLabel *label_sendInfo_rtt;
    // QLabel *label_31;
    // QLabel *label_sendInfo_state;
    // QLabel *label_33;
    // QLabel *label_sendInfo_stateKeep;
    // QLabel *label_36;
    // QLabel *label_sendInfo_dcong;
    // QLabel *label_40;
    // QLabel *label_sendInfo_fullrate;
    // QFormLayout *formLayout_2;
    // QLabel *label_28;
    // QLabel *label_sendInfo_currentPackage;
    // QLabel *label_30;
    // QLabel *label_sendInfo_totalPackege;
    // QLabel *label_29;
    // QLabel *label_sendInfo_progress;
    // QLabel *label_35;
    // QLabel *label_sendInfo_loss;
    // QLabel *label_32;
    // QLabel *label_sendInfo_dbase;
    // QLabel *label_38;
    // QLabel *label_sendInfo_drainsafe;
    // QLabel *label_27;
    // QLabel *label_26;
    vm->o_ccinput.use(this,[this]{
        auto input = vm->o_ccinput.get();
        ui->label_sendInfo_rtt->setText(QString::number(input.rtt,'f'));
        ui->label_sendInfo_currentPackage->setText(QString::number(input.chunkId));
        ui->label_sendInfo_totalPackege->setText(QString::number(input.totalChunks));
        ui->label_sendInfo_deliverRate->setText(QString::number(input.deliverRate,'f'));
        ui->label_sendInfo_loss->setNum((int)input.loss.size());
        ui->label_sendInfo_progress->setText(QString::number(input.chunkId/input.totalChunks*100)+"%");
    });
    vm->o_ccoutput.use(this,[this]{
        auto output = vm->o_ccoutput.get();
        ui->label_sendInfo_currentRate->setText(QString::number(output.rate,'f'));
        ui->label_sendInfo_fullrate->setText(QString::number(output.fullrate,'f'));
        ui->label_sendInfo_dbase->setNum(output.dbase);
        ui->label_sendInfo_dcong->setNum(output.dcong);
        ui->label_sendInfo_state->setNum((int)output.state);
    });
    // QTableWidget *tableWidget_deviceList;
    ui->tableView_deviceList->setModel(&vm->deviceModel);
    // QWidget *tab_remoteControl;
    // QGridLayout *gridLayout_9;
    // QPushButton *pushButton_remote_stop;
    connect(ui->pushButton_remote_stop,&QPushButton::clicked,vm,&ViewModel::on_stop_remote);
    // RemoteControlWidget *widget_remoteControl;
    connect(vm,&ViewModel::remoteControlEngineUpdated,this,[this](RemoteControlEngine *eng){
        ui->widget_remoteControl->deleteLater();
        QGridLayout *layout = (QGridLayout*)(ui->tab_remoteControl->layout());
        layout->addWidget(new RemoteControlWidget(this,eng),1,0); 
    });
    // QWidget *tab_proxy;
    // QGridLayout *gridLayout_6;
    // QTextBrowser *textBrowser_proxy;
    // QPushButton *pushButton_switchProxy;
    // QWidget *tab_settings;
    // QGridLayout *gridLayout_4;
    // QPushButton *pushButton_settings_save;
    // connect(ui->pushButton_settings_save,&QPushButton::clicked,vm,&ViewModel::on_settings_saved);//##### 移动到最后调用！！！
    // QScrollArea *scrollArea_settings;
    // QWidget *scrollAreaWidgetContents;
    // QGridLayout *gridLayout_8;
    // QLabel *label_18;
    // QLabel *label_11;
    // QHBoxLayout *horizontalLayout;
    // QPushButton *pushButton_settings_recordLog;
    /*vm->o_recordLogState.use(this,[this]{
        
    });*/
    // QPushButton *pushButton_settings_console;//###### 废弃
    // QLineEdit *lineEdit_settings_pwd;
    vm->o_pwd.use(ui->lineEdit_settings_pwd,[this]{
        ui->lineEdit_settings_pwd->setText(vm->o_pwd);
    });
    // QLineEdit *lineEdit_settings_githubPAT;//#######废弃
    // QLineEdit *lineEdit_settings_description;
    vm->o_description.use(ui->lineEdit_settings_description,[this]{
        ui->lineEdit_settings_description->setText(vm->o_description);
    });
    // QLabel *label_3;
    // QLabel *label_23;
    // QSpinBox *spinBox_settings_mqttPort;
    vm->o_mqttServer.use(ui->spinBox_settings_mqttPort,[this]{
        ui->spinBox_settings_mqttPort->setValue(vm->o_mqttServer.get().port);
    });
    // QLabel *label;
    // QLabel *label_8;
    // QLabel *label_22;
    // QPushButton *pushButton_settings_getDefaultPAT;//###### 废弃
    // QLineEdit *lineEdit_settings_username;
    vm->o_user_name.use(ui->lineEdit_settings_username,[this]{
        ui->lineEdit_settings_username->setText(vm->o_user_name);
    });
    // QCheckBox *checkBox_settings_disableNotice;
    vm->o_disableNoticeState.use(ui->checkBox_settings_disableNotice,[this]{
        ui->checkBox_settings_disableNotice->setChecked(vm->o_disableNoticeState);
    });
    // QLabel *label_9;
    // QCheckBox *checkBox_settings_recordLog;
    vm->o_recordLogState.use(ui->checkBox_settings_recordLog,[this]{
        ui->checkBox_settings_recordLog->setChecked(vm->o_recordLogState);
    });
    // QFrame *line;
    // QLabel *label_2;
    // QLabel *label_16;
    // QLineEdit *lineEdit_settings_gitubUser;//##### 废弃
    // QPushButton *pushButton_copyId;
    // QLabel *label_10;
    // QPushButton *pushButton_settings_requestUI;
    // QLabel *label_12;
    // QLabel *label_6;
    // QLabel *label_21;
    // QLabel *label_14;
    // QLabel *label_15;
    // QLabel *label_7;
    // QLabel *label_4;
    // QLabel *label_13;
    // QLabel *label_5;
    // QCheckBox *checkBox_settings_ipv6;
    vm->o_ipv6UsageState.use(ui->checkBox_settings_ipv6,[this]{
        ui->checkBox_settings_ipv6->setChecked(vm->o_ipv6UsageState);
    });
    // QLineEdit *lineEdit_settings_mqttServer;
    vm->o_mqttServer.use(ui->lineEdit_settings_mqttServer,[this]{
        ui->lineEdit_settings_mqttServer->setText(vm->o_mqttServer.get().ip);
    });
    // QComboBox *comboBox_settings_uiskin;//###### 废弃
    // QHBoxLayout *horizontalLayout_3;
    // QPushButton *pushButton_settings_mode_normal;
    connect(ui->pushButton_settings_mode_normal,&QPushButton::clicked,vm,&ViewModel::switchToNormal);
    // QPushButton *pushButton_settings_mode_dfhn;
    connect(ui->pushButton_settings_mode_dfhn,&QPushButton::clicked,vm,&ViewModel::switchToDfhn);
    // QComboBox *comboBox_settings_language;//###### 废弃
    // QLabel *label_34;
    // QCheckBox *checkBox_settings_stat;
    vm->o_statEnableState.use(ui->checkBox_settings_stat,[this]{
        ui->checkBox_settings_stat->setChecked(vm->o_statEnableState);
    });
    
    connect(ui->pushButton_settings_save,&QPushButton::clicked,vm,[this]{
        // OBS(bool,recordLogState);
        // OBS(bool,disableNoticeState);
        // OBS(QString,description);
        // OBS(bool,ipv6UsageState);
        // OBS(QString,user_name);
        // OBS(QString,pwd);
        // OBS(bool,statEnableState);
        // OBS(ipport,mqttServer);
        vm->o_recordLogState=ui->checkBox_settings_recordLog->isChecked();
        vm->o_disableNoticeState=ui->checkBox_settings_disableNotice->isChecked();
        vm->o_description=ui->lineEdit_settings_description->text();
        vm->o_ipv6UsageState=ui->checkBox_settings_ipv6->isChecked();
        vm->o_user_name=ui->lineEdit_settings_username->text();
        vm->o_pwd=ui->lineEdit_settings_pwd->text();
        vm->o_statEnableState=ui->checkBox_settings_stat->isChecked();
        vm->o_mqttServer=ipport{
            ui->lineEdit_settings_mqttServer->text(),
            static_cast<quint16>(ui->spinBox_settings_mqttPort->value())
        };
        
        vm->on_settings_saved();
    });    
    
    // QWidget *tab_userRules;
    // QGridLayout *gridLayout_5;
    // QTextBrowser *textBrowser_userRule_2;
    // QTextBrowser *textBrowser_userRule_1;
    // QWidget *tab_debug;
    // QGridLayout *gridLayout_3;
    // QPushButton *pushButton_debug1;
    connect(ui->pushButton_debug1,&QPushButton::clicked,vm,&ViewModel::on_debug);
    // QTextBrowser *textBrowser_debug1;
    // QTextEdit *textEdit_debug1;
    // QWidget *tab_route;
    // QGridLayout *gridLayout_10;
    // QCommandLinkButton *commandLinkButton_route_page6;
    connect(ui->commandLinkButton_route_page6,&QCommandLinkButton::clicked,vm,[this]{vm->o_currentPageIndex=6;});
    // QLabel *label_20;
    // QCommandLinkButton *commandLinkButton_route_page5;
    connect(ui->commandLinkButton_route_page5,&QCommandLinkButton::clicked,vm,[this]{vm->o_currentPageIndex=5;});
    // QCommandLinkButton *commandLinkButton_route_page3;
    connect(ui->commandLinkButton_route_page3,&QCommandLinkButton::clicked,vm,[this]{vm->o_currentPageIndex=3;});
    // QSpacerItem *verticalSpacer;
    // QCommandLinkButton *commandLinkButton_route_page2;
    connect(ui->commandLinkButton_route_page2,&QCommandLinkButton::clicked,vm,[this]{vm->o_currentPageIndex=2;});
    // QCommandLinkButton *commandLinkButton_route_help;
    connect(ui->commandLinkButton_route_help,&QCommandLinkButton::clicked,vm,&ViewModel::help);    
    // QMenuBar *menubar;
    // QMenu *menuFile;
    // QMenu *menuApplication_A;
    // QMenu *menu;
    // QStatusBar *statusBar;
    QLabel *primaryStatus = new QLabel;
    ui->statusBar->addPermanentWidget(primaryStatus);
    vm->o_status.use(primaryStatus,[primaryStatus, this]{
        primaryStatus->setText(vm->o_status);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}
