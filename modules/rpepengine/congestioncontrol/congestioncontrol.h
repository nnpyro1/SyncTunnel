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
    
    struct CongestionControlInput{
        QSet<quint32> loss;
        double rtt;
        quint32 chunkId;
        quint32 totalChunks;
    };
    
    struct CongestionControlOutput{
        double rate;        //速率,pps(package per second)
    };
    
    void reset();                               //重置状态
    void update(CongestionControlInput);        //更新
    CongestionControlOutput getOutput();        //获取输出
signals:
private:
    CongestionControlInput input;
    CongestionControlOutput output;
};

#endif // CONGESTIONCONTROL_H
