#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {
    timer.start();
}

void CongestionControl::reset()
{
    output=CongestionControlOutput();
    timer.restart();
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
    ++output.stateKeep;
    
    //算法
    if(ipt.rtt<output.dbase) output.dbase=ipt.rtt;
    //计算尾丢
    QList<quint32> tl;//尾丢
    {
        int last=ipt.end-1;
        for(auto it=ipt.loss.crbegin();it!=ipt.loss.crend();it++){
            if(last-*it < MAX_DROPTAIL_DIFFERENCE){
                tl.prepend(*it);
                last=*it;
            }
            else{
                break;
            }
        }
    }
    //删除随机丢包
    if(tl.size()==1){
        if(tl[0]==input.end-1||tl[0]==input.end-2){//随机丢包特征
            tl.clear();
        }
    }
    
    //切换状态
    switch(output.state){
    case Startup:{
        if(!tl.empty()){
            output.state=CongestionResponse;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    case Growth:{
        if(!tl.empty()){
            output.state=CongestionResponse;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    case CongestionResponse:{
        if(output.lastRttReportEnd<=input.end){//一个RTT过去了
            output.state=ProbeMaxRate1;
            output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    case ProbeMaxRate1:{
        if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
            output.state=ProbeMaxRate2;
            output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    case ProbeMaxRate2:{
        if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
            output.state=Drain;
            output.lastRttReportEnd=input.chunkId;
            //清除状态并EWMA
            output.dcong = output.probeMaxRtt * PROBE_RTT_WEIGHT + output.dcong * (1-PROBE_RTT_WEIGHT);
            output.fullrate = output.probeMaxRate * PROBE_RATE_WEIGHT + output.fullrate * (1-PROBE_RATE_WEIGHT);
            output.stateKeep=0;
            output.probeMaxRate=0;
            output.probeMaxRtt=0;
        }
        break;
    }
    case Drain:{
        if(input.end>=(quint32)output.lastRttReportEnd){//一个RTT过去了
            output.state=Growth;
            output.lastRttReportEnd=input.chunkId;
            output.stateKeep=0;
        }
        break;
    }
    case SafeGrowth:{
        if(!tl.empty()){
            output.state=CongestionResponse;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    }
    
    //算法核心计算
    switch(output.state){
    case Startup:{
        if(input.end>=(quint32)output.lastRttReportEnd){//大于一个RTT
            output.lastRttReportEnd=input.chunkId;
            output.rate=output.rate*STARTUP_GAIN;
        }
        if(true/*output.fullrate==0*/){//初次初始化rullrate,后续禁止
            Q_ASSERT(input.elapsedTimes.contains(input.end));
            Q_ASSERT(input.elapsedTimes.contains(input.start));
            // double deliverRate = (input.end-input.start-tl.size()) / (input.elapsedTimes[input.end]-input.elapsedTimes[input.start]) * 1000;
            // output.fullrate=deliverRate;
            if(input.deliverRate>output.fullrate){
                output.fullrate=input.deliverRate;
            }
        }
        break;
    }
    case Growth:{
        // double m = 2.687368213 * ( (output.dcong-output.dbase) / (output.dbase*GROWTH_ELAPSE) );
        // double q = (input.rtt-output.dbase)/(output.dcong-output.dbase);//队列比例
        // if(q<=0.5){
        //     output.rate = output.fullrate + output.fullrate * ( m * pow(32,q-0.5) );
        // }
        // else{
        //     output.rate = output.fullrate + output.fullrate * ( m / pow(32,q-0.5) );
        // }
        if(input.rtt<=output.dcong){
            output.rate=output.fullrate+output.fullrate * GROWTH_K * ( 1 - (input.rtt-output.dbase)/(output.dcong-output.dbase) ) + GROWTH_B;
        }
        else{
            output.rate=output.fullrate+output.fullrate*GROWTH_B;
        }
        //侦测异常
        output.growthQueueFracWindow.insert(input.chunkId,(input.rtt-output.dbase)/(output.dcong-output.dbase));
        output.growthQueueFracWindow.erase(output.growthQueueFracWindow.begin(),output.growthQueueFracWindow.lowerBound(input.end));//删除旧值
        if(output.growthQueueFracWindow.size()>=(input.chunkId-input.end) && (input.rtt-output.dbase)/(output.dcong-output.dbase)-output.growthQueueFracWindow.first()<0){
            //可以通融在此切换
            output.state=SafeGrowth;
            output.growthQueueFracWindow.clear();
        }
        break;
    }
    case CongestionResponse:{
        output.rate = output.fullrate * CONG_GAIN;
        break;
    }
    case ProbeMaxRate1:
    case ProbeMaxRate2:{
        double q = (input.rtt-output.dbase)/(output.dcong-output.dbase);//队列比例
        //调速
        output.rate=output.fullrate;
        if(q<PROBE_MIN_BUF){
            output.rate=output.fullrate*PROBE_UP_GAIN;
        }
        if(!tl.empty()){
            output.rate=output.fullrate*PROBE_DOWN_GAIN;
        }
        //侦测
        Q_ASSERT(input.elapsedTimes.contains(input.end));
        Q_ASSERT(input.elapsedTimes.contains(input.start));
        // double deliverRate = (input.end-input.start-tl.size()) / (input.elapsedTimes[input.end]-input.elapsedTimes[input.start]) * 1000;
        output.probeMaxRate = qMax(input.deliverRate,output.probeMaxRate);
        output.probeMaxRtt = qMax(input.rtt,output.probeMaxRtt);
        break;
    }
    case Drain:{
        output.rate = qMax( output.fullrate - ( output.fullrate*(input.rtt-output.dbase) - output.fullrate*(output.dcong-output.dbase)*DRAIN_MIN_BUF ) / output.dbase , DRAIN_MIN_RATE_FRAC * output.fullrate);
        break;
    }
    case SafeGrowth:{
        if(output.lastRttReportEnd<=input.end){output.rate=output.rate*SAFEGROWTH_GAIN;}
        break;
    }
    }
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
