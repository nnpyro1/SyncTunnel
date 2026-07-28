#ifndef CONGESTIONCONTROL_H
#define CONGESTIONCONTROL_H

#include <QObject>
#include <QSet>
#include     <general.h>
#include <core/basic/utils.h>

class CongestionControl : public QObject
{
    Q_OBJECT
public:
    explicit CongestionControl(QObject *parent = nullptr);
    
    enum State{
        Startup,
        CongestionResponse,
        DrainHalf,
        Drain,
        Growth,
    };
    
    struct CongestionControlInput{
        QList<quint32> loss;//要求有序
        double rtt;
        // quint32 chunkId;
        // quint32 totalChunks;
        quint32 start;
        quint32 end;
        // quint64 timeToLastReport;//距离上一个统计的时间
        QMap<quint32,double> elapsedTimes;//多包间隔时间
    };
    
    struct CongestionControlOutput{
        double rate         =10;        //速率,pps(package per second)
        State state         =Startup;   //状态机状态
        double crtt         = 0;        //拥塞时的rtt
        double brtt         = 0;        //基准rtt
        double fullrate     = 0;        //满载速率，瓶颈节点的出队速度
        double maxrate      = 0;        //最大速率，节点上次发送达到的最大速率，大于满载速率
        double halfdraincnt = 0;        //排空队列50%需要用到的RTT计数
    };
    
    void reset();                               //重置状态
    void update(CongestionControlInput);        //更新
    CongestionControlOutput getOutput();        //获取输出
    
    static const int MAX_DROPTAIL_DIFFERENCE = 2;
    static const int STARTUP_RATE_GAIN = 2;
    static const int DRAIN_RATE_GAIN  = 2;
signals:
private:
    CongestionControlInput input;
    CongestionControlOutput output;
};

#endif // CONGESTIONCONTROL_H
