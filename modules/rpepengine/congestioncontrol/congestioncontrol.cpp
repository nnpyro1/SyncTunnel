#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {}

void CongestionControl::reset()
{
    
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
    ++output.stateKeep;
    
    //算法
    double lastUpdate = timer.isValid()?(timer.nsecsElapsed()/1.e6):0;
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
    
    //状态切换
    if(output.state==Startup){
        if(output.dbase==0){
            output.dbase=input.rtt;
        }
        if(tl.size()>0){
            output.stateKeep=0;
            output.state=CongestionResponse;
            output.dcong = input.rtt;
        }
    }
    if(output.state==CongestionResponse){
        if(tl.size()==0){
            output.stateKeep=0;
            output.state=Drain;
            output.drainsafe=(MAX_SAFE_QUEUE_LEN * (1-DRAIN_QUEUE_LEN)) * (output.fullrate * (1-DRAIN_GAIN) * input.rtt);//计算最大安全次数
        }
    }
    if(output.state==Drain){
        if(output.stateKeep >= output.drainsafe ||
            input.rtt <= DRAIN_QUEUE_LEN * output.dcong +(1-DRAIN_QUEUE_LEN) * output.dbase){
            output.stateKeep=0;
            output.state=Growth;
        }
    }
    if(output.state==Growth){
        if(tl.size()>0){
            output.state=CongestionResponse;
            output.stateKeep=0;
            output.dcong = input.rtt;
        }
    }
    
    //核心算法
    if(output.state==Startup){
        // output.rate=output.rate*1.5;
        output.rate+=INIT_INCREMENT;
    }
    if(output.state==CongestionResponse){
        // double tc = lastUpdate;//拥塞时间
        // double fullrate = (output.rate * tc - tl.size()) / tc;//满载速率计算公式
        Q_ASSERT(input.elapsedTimes.contains(input.end));
        Q_ASSERT(input.elapsedTimes.contains(input.lastEnd));
        double fullrate = (input.end - input.lastEnd - tl.size())/((input.elapsedTimes[input.end] - input.elapsedTimes[input.lastEnd])/1000.);//满载速率计算
        if(output.fullrate==0){//没有初始化
            output.fullrate=output.rate=fullrate;
        }
        else{
            output.fullrate = output.fullrate * (1-MAXRATE_WEIGHT) + fullrate * MAXRATE_WEIGHT;//加权平均
            output.rate=fullrate;
        }
    }
    if(output.state==Drain){
        output.rate=output.fullrate*DRAIN_GAIN;
    }
    if(output.state==Growth){
        output.rate = (1-GROWTH_START)*output.fullrate+qSqrt(output.stateKeep)+GROWTH_START*output.fullrate;
    }
    
    timer.restart();
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
