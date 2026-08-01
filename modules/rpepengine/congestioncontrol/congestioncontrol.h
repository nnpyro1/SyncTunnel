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
        quint32 lastEnd;
        quint32 lastSend;
        //以下是没用的
        quint32 chunkId;
        quint32 totalChunks;
    };
    
    struct CongestionControlOutput{
        double rate         =10;        //速率,pps(package per second)
        State state         =Startup;   //状态机状态
        int stateKeep       =0;         //状态持续次数，按需使用，不必勉强
        double dbase        =0;         //基准RTT，需在Startup初始化，Drain更新
        int drainsafe       =0;         //为了防止基准RTT变化导致状态锁死而设置的最大排空时间 单位：次
        double dcong        =0;         //拥塞RTT
        double fullrate     =0;         //网络满载速率
    };
    
    void reset();                               //重置状态
    void update(CongestionControlInput);        //更新
    CongestionControlOutput getOutput();        //获取输出
    
    constexpr static const int MAX_DROPTAIL_DIFFERENCE = 2;
    constexpr static const int INIT_INCREMENT = 800;
    // constexpr static const int DRAIN_RATE_GAIN  = 2;
    constexpr static const double DRAIN_GAIN = 0.8;
    constexpr static const double DRAIN_QUEUE_LEN = 0.2;//排空需要的队列长度 百分数
    constexpr static const int MAX_SAFE_QUEUE_LEN = 850;//算法工作的最大安全缓冲区，防止基准RTT变化导致的状态锁死
    constexpr static const double MAXRATE_WEIGHT = 0.5;
    constexpr static const double GROWTH_START = 0.95;
signals:
private:
    CongestionControlInput input;
    CongestionControlOutput output;
    QElapsedTimer timer;
};

#endif // CONGESTIONCONTROL_H
