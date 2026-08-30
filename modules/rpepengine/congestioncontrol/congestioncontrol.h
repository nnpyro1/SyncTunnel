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
        // Startup,
        // CongestionResponse,
        // Drain,
        // Growth,
        // Startup,
        // Growth,
        // CongestionResponse,
        // ProbeMaxRate1,
        // ProbeMaxRate2,
        // Drain,
        // SafeGrowth,
        StartupUp,
        StartupDown,
        ProbeMaxRateGrowth,
        ProbeMaxRateUp,
        ProbeMaxRateDown,
        Push
    };
    
    struct CongestionControlInput{
        QList<quint32> loss;//要求有序
        double rtt;
        // quint32 chunkId;
        // quint32 totalChunks;
        quint32 start;
        quint32 end;
        // quint64 timeToLastReport;//距离上一个统计的时间
        QHash<quint32,double> elapsedTimes;//多包间隔时间
        quint32 lastEnd;
        quint32 lastSend;
        double deliverRate;
        //以下是没用的
        quint32 chunkId;
        quint32 totalChunks;
    };
    
    struct CongestionControlOutput{
        // double rate         =10;        //速率,pps(package per second)
        // State state         =Startup;   //状态机状态
        // int stateKeep       =0;         //状态持续次数，按需使用，不必勉强
        // double dbase        =0;         //基准RTT，需在Startup初始化，Drain更新
        // int drainsafe       =0;         //为了防止基准RTT变化导致状态锁死而设置的最大排空时间 单位：次
        // double dcong        =0;         //拥塞RTT
        // double fullrate     =0;         //网络满载速率
        double rate         =10;        //速率，pps
        State state         =StartupUp; //状态
        int stateKeep       =0;         //状态持续次数
        double dbase        =1048576;   //RTprop,d0
        double dcong        =0;         //拥塞满载速率
        double fullrate     =0;         //满载速率
        quint32 lastRttReportEnd=0;     //上一个RTT的Report的end，用于区分是否是一个新的RTT
        
        // uint probeTimeout   =0;         //ProbeMaxRate1/2的超时时间戳
        double probeMaxRate =0;         //ProbeMaxRate1/2获取的最大速率
        double probeMaxRtt  =0;         //ProbeMaxRate1/2获取的最大延迟
        // QMap<quint32,double> growthQueueFracWindow;//增长期间的q的窗口,key=过期时间的end，value=q
        int probeRttCount   =0;         //PMR的时候持续的RTT数量
        double probeRdFrac  =0;         //PMR的时候EWMA的队列变化率
        double pushQueueFrac=0;         //EWMA过的队列占用率
        quint32 pmrgEnterEnd=0;         //PMR Growth进入时的Report end
        quint32 pmrgEnterId =0;         //PMR Growth进入时的包号
        double rttAvgRd     =0;         //RTT平均的交付速率
    };
    
    void reset();                               //重置状态
    void update(CongestionControlInput);        //更新
    CongestionControlOutput getOutput();        //获取输出
    
    constexpr static const int MAX_DROPTAIL_DIFFERENCE = 2;
    // constexpr static const int INIT_INCREMENT = 800;
    // // constexpr static const int DRAIN_RATE_GAIN  = 2;
    // constexpr static const double DRAIN_GAIN = 0.8;
    // constexpr static const double DRAIN_QUEUE_LEN = 0.2;//排空需要的队列长度 百分数
    // constexpr static const int MAX_SAFE_QUEUE_LEN = 850;//算法工作的最大安全缓冲区，防止基准RTT变化导致的状态锁死
    // constexpr static const double MAXRATE_WEIGHT = 0.5;
    // constexpr static const double GROWTH_START = 0.95;
    constexpr static const double STARTUP_GAIN = 1.5;
    // constexpr static const double PROBE_UP_GAIN = 1.2;
    // constexpr static const double PROBE_DOWN_GAIN = 0.85;
    // constexpr static const double CONG_GAIN = 0.85;
    constexpr static const double DRAIN_MIN_BUF = 0.35;
    // constexpr static const double ALPHA_FAIR = 0.01;
    // constexpr static const double BETA_FAIR = 1;
    // constexpr static const int    PROBE_TIMEOUT = 3000;
    // constexpr static const int    GROWTH_ELAPSE = 3;
    // constexpr static const double GROWTH_K = 0.45;
    // constexpr static const double GROWTH_B = 0.05;
    // constexpr static const double PROBE_MIN_BUF = 0.6;
    constexpr static const double PROBE_RTT_WEIGHT = 0.8;
    constexpr static const double PROBE_RATE_WEIGHT = 0.8;
    constexpr static const double DRAIN_MIN_RATE_FRAC = 0.25;
    constexpr static const double SAFEGROWTH_GAIN = 1.08;
    // constexpr static const double DCONG_WEIGHT = 0.2;
    // constexpr static const double
    constexpr static const double STARTUP_DOWN_GAIN = 0.6;
    constexpr static const int PMR_UP_TIMEOUT = 3;
    constexpr static const double PMR_UP_GAIN = 1.25;
    constexpr static const double PMR_UP_RATE_FRAC = 0.01;
    constexpr static const double PMR_DOWN_BUF = 0.3;
    constexpr static const double PUSH_DELTA = 0.1;
    constexpr static const double PMR_UP_EWMA_WEIGHT = 0.08;
    constexpr static const double PUSH_QUEUE_EWMA_WEIGHT = 0.08;
    constexpr static const double PMR_UP_ALLOW_RATE_LOW = 0.1;
    constexpr static const double PMR_UP_DELIVERRATE_EWMA_WEIGHT = 0.3;
    signals:
private:
    CongestionControlInput input;
    CongestionControlOutput output;
    QElapsedTimer timer;
};

#endif // CONGESTIONCONTROL_H
