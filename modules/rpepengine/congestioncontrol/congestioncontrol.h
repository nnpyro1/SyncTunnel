#ifndef CONGESTIONCONTROL_H
#define CONGESTIONCONTROL_H

#include <QObject>

class CongestionControl : public QObject
{
    Q_OBJECT
public:
    explicit CongestionControl(QObject *parent = nullptr);
    
    struct CongestionControlInput{
        
    };
    
    struct CongestionControlOutput{
        double rate;        //速率,pps(package per second)
    };
    
    void reset();                               //重置状态
    void update(CongestionControlInput);        //更新
    CongestionControlOutput getOutput();        //获取输出
signals:
};

#endif // CONGESTIONCONTROL_H
