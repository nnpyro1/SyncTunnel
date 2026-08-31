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
    //仅用于测试！！！
    // {
    //     output.rate=4000;
    //     return;
    // }
    
    input=ipt;
    ++output.stateKeep;
    
    //算法
    if(ipt.rtt<output.dbase) output.dbase=ipt.rtt;
    //计算尾丢
    QList<quint32> tl;//尾丢
    {
        int last=ipt.end-1;
        tl.resize(ipt.loss.size());//先用最大大小
        auto data=tl.data();//性能优化
        quint32 idx=0;
        for(auto it=ipt.loss.crbegin();it!=ipt.loss.crend();it++){
            auto i=*it;
            if(last-i < MAX_DROPTAIL_DIFFERENCE){
                // tl.prepend(*it);
                data[idx++]=i;
                last=i;
            }
            else{
                break;
            }
        }
        tl.resize(idx);//设回正常大小
        std::reverse(tl.begin(),tl.end());
    }
    //删除随机丢包
    if(tl.size()==1){
        if(tl[0]==input.end-1||tl[0]==input.end-2){//随机丢包特征
            tl.clear();
        }
    }
    
    //切换状态
    switch(output.state){
    case StartupUp:{
        if(!tl.empty()){
            output.state=StartupDown;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId+1;
        }
        break;
    }
    case StartupDown:{
        if(output.lastRttReportEnd<=input.end){
            output.state=ProbeMaxRateGrowth;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId+1;
            output.pmrgEnterEnd=input.end;
            output.pmrgEnterId=input.chunkId;
        }
        break;
    }
    case ProbeMaxRateGrowth:{
        if(!tl.empty()){
            output.state=ProbeMaxRateDown;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId+1;
            break;
        }
        if(output.lastRttReportEnd<=input.end){//一个RTT过去了
            output.state=ProbeMaxRateUp;
            // output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
            output.stateKeep=0;
            output.lastRttReportEnd=input.chunkId+1;
        }
        break;
    }
    case ProbeMaxRateUp:{
        // if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
        //     output.state=ProbeMaxRate2;
        //     output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
        //     output.stateKeep=0;
        //     output.lastRttReportEnd=input.chunkId+1;
        // }
        // if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
        //     output.state=Drain;
        //     output.lastRttReportEnd=input.chunkId+1;
        //     //清除状态并EWMA
        //     output.dcong = output.probeMaxRtt * PROBE_RTT_WEIGHT + output.dcong * (1-PROBE_RTT_WEIGHT);
        //     output.fullrate = output.probeMaxRate * PROBE_RATE_WEIGHT + output.fullrate * (1-PROBE_RATE_WEIGHT);
        //     output.stateKeep=0;
        //     output.probeMaxRate=0;
        //     output.probeMaxRtt=0;
        // }
        if(output.lastRttReportEnd<=input.end){
            output.lastRttReportEnd=input.chunkId+1;
            output.probeRttCount++;
            if(/*output.probeRdFrac<PMR_UP_RATE_FRAC*/(output.rttAvgRd<output.probeMaxRate*(1-PMR_UP_ALLOW_RATE_LOW)&&output.probeRttCount>1) ||
                output.probeRttCount>PMR_UP_TIMEOUT
                ){
                output.state=/*Push*/ input.deliverRate<(output.rate+output.fullrate)/2?Push:ProbeMaxRateGrowth;
                if(output.state==ProbeMaxRateGrowth){
                    output.pmrgEnterEnd=input.end;
                    output.pmrgEnterId=input.chunkId;
                }
                //清除状态并EWMA
                output.dcong = output.probeMaxRtt * PROBE_RTT_WEIGHT + output.dcong * (1-PROBE_RTT_WEIGHT);
                output.fullrate = output.probeMaxRate * PROBE_RATE_WEIGHT + output.fullrate * (1-PROBE_RATE_WEIGHT);
                output.stateKeep=0;
                output.probeMaxRate=0;
                output.probeMaxRtt=0;
                output.probeRttCount=0;
                output.probeRdFrac=0;
            }
            output.rttAvgRd=0;
        }
        if(!tl.empty()){
            output.state=ProbeMaxRateDown;
            //清除状态并EWMA
            output.dcong = output.probeMaxRtt * PROBE_RTT_WEIGHT + output.dcong * (1-PROBE_RTT_WEIGHT);
            output.fullrate = output.probeMaxRate * PROBE_RATE_WEIGHT + output.fullrate * (1-PROBE_RATE_WEIGHT);
            output.stateKeep=0;
            output.probeMaxRate=0;
            output.probeMaxRtt=0;
            output.probeRttCount=0;
            output.probeRdFrac=0;
        }
        break;
    }
    case ProbeMaxRateDown:{
        // if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
        //     output.state=Drain;
        //     output.lastRttReportEnd=input.chunkId+1;
        //     //清除状态并EWMA
        //     output.dcong = output.probeMaxRtt * PROBE_RTT_WEIGHT + output.dcong * (1-PROBE_RTT_WEIGHT);
        //     output.fullrate = output.probeMaxRate * PROBE_RATE_WEIGHT + output.fullrate * (1-PROBE_RATE_WEIGHT);
        //     output.stateKeep=0;
        //     output.probeMaxRate=0;
        //     output.probeMaxRtt=0;
        // }
        if(output.lastRttReportEnd<=input.end){
            output.lastRttReportEnd=input.end+1;
            output.stateKeep=0;
            output.state=Push;
        }
        break;
    }
    case Push:{
        // if(input.end>=(quint32)output.lastRttReportEnd){//一个RTT过去了
        //     output.growthQueueFracWindow.clear();
        //     output.state=Growth;
        //     output.lastRttReportEnd=input.chunkId+1;
        //     output.stateKeep=0;
        // }
        if(output.lastRttReportEnd<=input.end){
            output.lastRttReportEnd=input.end+1;
            if(output.pushQueueFrac<PMR_DOWN_BUF-PUSH_DELTA ||
                output.pushQueueFrac>PMR_DOWN_BUF+PUSH_DELTA){
                output.state=ProbeMaxRateGrowth;
                output.stateKeep=0;
                output.pmrgEnterEnd=input.end;
                output.pmrgEnterId=input.chunkId;
            }
        }
        break;
    }
    // case SafeGrowth:{
    //     if(!tl.empty()){
    //         output.state=CongestionResponse;
    //         output.stateKeep=0;
    //         output.lastRttReportEnd=input.chunkId+1;
    //     }
    //     break;
    // }
    }
    
    //算法核心计算
    switch(output.state){
    case StartupUp:{
        if(input.end>=(quint32)output.lastRttReportEnd){//大于一个RTT
            output.lastRttReportEnd=input.chunkId+1;
            output.rate=output.rate*STARTUP_GAIN;
        }
        if(true/*output.fullrate==0*/){//初次初始化rullrate,后续禁止
            Q_ASSERT(input.elapsedTimes.contains(input.end));
            Q_ASSERT(input.elapsedTimes.contains(input.start));
            // double deliverRate = (input.end-input.start-tl.size()) / (input.elapsedTimes[input.end]-input.elapsedTimes[input.start]) * 1000;
            // output.fullrate=deliverRate;
            if(input.deliverRate>output.fullrate || output.pmrgEnterId == output.pmrgEnterEnd){
                output.fullrate=input.deliverRate;
            }
            output.dcong=qMax(output.dcong,input.rtt);
        }
        break;
    }
    case StartupDown:{
        output.rate = output.fullrate * STARTUP_DOWN_GAIN;
        output.probeMaxRate = qMax(input.deliverRate,output.probeMaxRate);
        output.probeMaxRtt = qMax(input.rtt,output.probeMaxRtt);
        break;
    }
    case ProbeMaxRateGrowth :{
        // // double m = 2.687368213 * ( (output.dcong-output.dbase) / (output.dbase*GROWTH_ELAPSE) );
        // // double q = (input.rtt-output.dbase)/(output.dcong-output.dbase);//队列比例
        // // if(q<=0.5){
        // //     output.rate = output.fullrate + output.fullrate * ( m * pow(32,q-0.5) );
        // // }
        // // else{
        // //     output.rate = output.fullrate + output.fullrate * ( m / pow(32,q-0.5) );
        // // }
        // if(input.rtt<=output.dcong){
        //     output.rate=output.fullrate+output.fullrate * GROWTH_K * ( 1 - (input.rtt-output.dbase)/(output.dcong-output.dbase) ) + GROWTH_B;
        // }
        // else{
        //     output.rate=output.fullrate+output.fullrate*GROWTH_B;
        // }
        // output.probeMaxRate = qMax(input.deliverRate,output.probeMaxRate);
        // output.probeMaxRtt = qMax(input.rtt,output.probeMaxRtt);
        // //侦测异常
        // output.growthQueueFracWindow.insert(input.chunkId,(input.rtt-output.dbase)/(output.dcong-output.dbase));
        // output.growthQueueFracWindow.erase(output.growthQueueFracWindow.begin(),output.growthQueueFracWindow.lowerBound(input.end));//删除旧值
        // if(output.growthQueueFracWindow.size()>=(input.chunkId-input.end) && !output.growthQueueFracWindow.empty() && (input.rtt-output.dbase)/(output.dcong-output.dbase)-output.growthQueueFracWindow.first()<0){
        //     //可以通融在此切换
        //     output.state=SafeGrowth;
        //     output.growthQueueFracWindow.clear();
        // }
        // break;
        output.rate=output.fullrate + (1.*input.end-output.pmrgEnterEnd)/(output.pmrgEnterId-output.pmrgEnterEnd)*PMR_UP_GAIN*output.fullrate;
        if(output.rate<=0 || output.pmrgEnterId==output.pmrgEnterEnd){//乱序和重复
            output.rate=output.fullrate;
        }
        break;
    }
    case ProbeMaxRateUp:{
        output.rate=PMR_UP_GAIN*output.fullrate;
        output.probeMaxRate=qMax(output.probeMaxRate,input.deliverRate);
        output.probeMaxRtt=qMax(output.probeMaxRtt,input.rtt);
        output.probeRdFrac=output.probeRdFrac*(1-PMR_UP_EWMA_WEIGHT)+input.deliverRate*PMR_UP_EWMA_WEIGHT;
        if(output.rttAvgRd==0){
            output.rttAvgRd=input.deliverRate;
        }
        else{
            output.rttAvgRd=input.deliverRate*(1-PMR_UP_DELIVERRATE_EWMA_WEIGHT)+output.rttAvgRd*PMR_UP_DELIVERRATE_EWMA_WEIGHT;
        }
        break;
    }
    case ProbeMaxRateDown:{
        output.rate = qMax( output.fullrate - ( output.fullrate*(input.rtt-output.dbase) - output.fullrate*(output.dcong-output.dbase)*PMR_DOWN_BUF ) / output.dbase , DRAIN_MIN_RATE_FRAC * output.fullrate);
        break;
    }
    case Push:{
        double q = (input.rtt-output.dbase)/(output.dcong-output.dbase);//队列比例
        output.pushQueueFrac=(1-PUSH_QUEUE_EWMA_WEIGHT)*output.pushQueueFrac+PUSH_QUEUE_EWMA_WEIGHT*q;
        //调速
        output.rate=output.fullrate;
        // if(q<PROBE_MIN_BUF){
        //     output.rate=output.fullrate*PROBE_UP_GAIN;
        // }
        // if(!tl.empty()){
        //     output.rate=output.fullrate*PROBE_DOWN_GAIN;
        // }
        //侦测
        // Q_ASSERT(input.elapsedTimes.contains(input.end));
        // Q_ASSERT(input.elapsedTimes.contains(input.start));
        // // double deliverRate = (input.end-input.start-tl.size()) / (input.elapsedTimes[input.end]-input.elapsedTimes[input.start]) * 1000;
        // output.probeMaxRate = qMax(input.deliverRate,output.probeMaxRate);
        // output.probeMaxRtt = qMax(input.rtt,output.probeMaxRtt);
        break;
    }
    }
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
