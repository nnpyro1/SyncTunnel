#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {}

void CongestionControl::reset()
{
    
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
    
    //算法
    double lastUpdate = timer.isValid()?(timer.nsecsElapsed()/1.e6):0;
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
            output.state=CongestionResponse;
            output.dcong = input.rtt;
        }
    }
    if(output.state==CongestionResponse){
        if(tl.size()==0){
            output.state=Drain;
        }
    }
    if(output.state==Drain){
        if(output.stateKeep >= output.drainsafe ||
            input.rtt <= DRAIN_QUEUE_LEN * output.dcong +(1-DRAIN_QUEUE_LEN) * output.dbase){
            output.state=Growth;
        } 
    }
    if(output.state==Growth){
        if(tl.size()>0){
            output.state=CongestionResponse;
            output.dcong = input.rtt;
        }
    }
    
    //核心算法
    if(output.state==Startup){
        output.rate=output.rate*2;
    }
    if(output.state==CongestionResponse){
        double tc = lastUpdate;//拥塞时间
        output.rate = output.fullrate = (output.rate * tc - tl.size()) / tc;
    }
    if(output.state==Drain){
        
    }
    if(output.state==Growth){
        
    }
    
    timer.restart();
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
