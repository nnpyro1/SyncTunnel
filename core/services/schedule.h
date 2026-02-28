#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QObject>
#include <QSet>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QTimer>
#include <QDataStream>


class Schedule : public QObject
{
    Q_OBJECT
public:
    explicit Schedule(QObject *parent = nullptr);
    
    enum Type{
        t_time,         //设置时间。需要重复
        t_datetime,     //设置几月几日几点几分
    };

    enum Repeat{
        rp_Monday           = 0b00000001,
        rp_Tuesday          = 0b00000010,
        rp_Wednesday        = 0b00000100,
        rp_Thursday         = 0b00001000,
        rp_Friday           = 0b00010000,
        rp_Saturday         = 0b00100000,
        rp_Sunday           = 0b01000000,
    };
    
public://公共函数
    //单项设置&读取
    void setType(Type tp);                               //设置类型
    void setRepeat(Repeat rep);                         //设置重复
    void setTime(QTime tp);                              //设置每天时间
    void setDateTime(QDateTime tp);                      //设置日期时间
    inline Type type()      {return tp;}
    inline Repeat repeat()  {return rep;}
    inline QTime time()     {return tm;}
    inline QDateTime datetime(){return dtm;}
    
    //组合设置
    void set(QDateTime tp);                              //自动设置 指定具体日期
    void set(QTime tp,Repeat repeat);                    //自动设置 指定重复+时间
    void set(long sec);                                 //自动设置 在sec秒后触发
    
    //属性
    void setEnabled(bool enabled);                      //是否启用
    bool isEnabled();
    void setAutoTrigger(bool autoTrigger);              //是否自动触发
    bool autoTrigger();
    void setInterval(int it);                           //自动触发间隔
    int  interval();
    
    //触发
    bool checkTrigger();
    
    //其他
    QString toString();
    friend QDataStream &operator<<(QDataStream &stream,const Schedule &s);
    friend QDataStream &operator>>(QDataStream &stream,Schedule &s);
    
signals:
    void triggered();                                   //触发
    
private://私有变量
    Type tp;
    Repeat rep;
    QTime tm;
    QDateTime dtm;
    QTimer *timer = nullptr;
    bool enabled = true;
    int interval_;
};

#endif // SCHEDULE_H
