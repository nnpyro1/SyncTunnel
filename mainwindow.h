#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QMap>
#include <QTimer>
#include <QDropEvent>
#include "modules/communication/communication.h"
#include "modules/storage/storage.h"
#include "modules/signalling/signalling.h"
#include <QCloseEvent>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; class Dialog; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef Communication::ipport ipport; 
public://公有函数
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void show_dir();                            //显示目录到Widget上
    void send(QByteArray msg,bool e=1);         //自动加密msg并发送给所有client,e标识是否需要加密
    QByteArray encode(QByteArray msg);          //加密msg并返回密文
    QByteArray decode(QByteArray msg);          //解密msg并返回解密后的值
    QString mergeFile(QDir folder);             //合并文件
    void sendFile();                            //自动分包并发送
    void releaseFile(QString msg);              //释放文件
    void savePower();                           //省电模式
    
private slots://槽函数
    void on_folder_change();                    //当前目录改变（双击打开）
    void on_rightclick();                       //目录被右击
    void on_readyRead();                        //communication模块接受到新的消息
    void on_request_resend();                   //请求重传文件
    void on_settings_saved();                   //设置的保存按钮被点击
    void on_hangup();                           //当文件挂起
    void on_download();                         //当文件下载
    void on_proxy();                            //当开始/停止网络加速
    
    void on_pushButton_debug1_clicked();//当调试
    
protected://继承
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    
private://模块
    Communication *m_communication;
//    Request *request;
    Storage *m_storage;
    Signalling *m_signalling;
    
private://私有变量
    Ui::MainWindow *ui;
    ipport public_ip;
    QDir current_dir;
    QList<ipport> clients;
    QString user_name,pwd;
    QMap<int,QString> currentFileMap;//当前正在传输的文件列表
    int currentFileTotal;//文件传输总数
    QTimer timer_fileResend;
    bool is_uploading = false;
    QStringList chunks;//文件区块
    QTimer timer_is_uploading;
    QLabel *label_status;
    QTimer timer_clear_currentFileMap;
    ipport mqtt_server;
    QString user_github_name;
    QString user_github_PAT;
    QProcess *process_proxy;//代理进程
    QProcess *process_proxy_ui;//代理UI进程
    Ui::Dialog *ui_dialog_diviceList;
    bool is_accept_shutdown = false;
    QWidget *widget_savePower=nullptr;//省电模式窗口
    QTimer timer_savePower;//省电模式刷新定时器
    QTimer timer_savePower_finish;//省电模式结束定时器
    
private:
    
};
#endif // MAINWINDOW_H
