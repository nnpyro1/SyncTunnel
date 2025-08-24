#include <QObject> 
#include <qmqttclient.h>
#include "../communication/communication.h"


class Signalling : public QObject
{
    Q_OBJECT
public:
    Signalling();
    virtual ~Signalling();
    
public://公有函数
    void connectToHost(Communication::ipport host);                     //联接到host指定的主机并等待到联接成功，不会订阅Topic
    QMqttSubscription *subscribe(QString topic);                        //订阅topic
    
    void setUser(const Communication::ipport user,QString name);        //设置用户的ipport。来源应该是Communication的stun
    void setPwd(const QByteArray p);                                    //设置密码
    QList<Communication::ipport> getUserList();                         //发送消息获取用户列表
    void exit();                                                        //退出    这个函数会销毁联接状态，需要在对象销毁的时候再用
    
private slots://私有槽
    void on_msg(QMqttMessage mqttMsg);                                  //收到消息
    
signals://信号
//    void private_on_user_list_finished(QPrivateSignal);
    void on_userlist_updata(QList<Communication::ipport> list);         //用户列表
    
private://私有函数
    void send_msg_to_get_user_list(int c);
    
private://私有对象&变量
    QMqttClient *client;
    QMqttSubscription *subscription;
    Communication::ipport user;//用户
    QList<Communication::ipport> user_list;//在线用户列表
    QByteArray pwd = QByteArray("SyncTunnel_defaultPWD_123456");
    QString topicName;
    bool is_waiting_userList = false;
    QString user_name;
};
