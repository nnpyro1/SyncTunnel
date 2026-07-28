#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {}

void CongestionControl::reset()
{
    
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
    
    //算法
    //计算尾丢
    QList<quint32> td;//尾丢
    {
        int last=ipt.end-1;
        for(auto it=ipt.loss.crbegin();it!=ipt.loss.crend();it++){
            if(last-*it < MAX_DROPTAIL_DIFFERENCE){
                td.prepend(*it);
                last=*it;
            }
            else{
                break;
            }
        }
    }
    //切换状态，初始化
    if(output.state==Startup){
        if(!td.empty()){
            output.state=CongestionResponse;
            output.crtt=ipt.rtt;
            output.maxrate=output.rate;
        }
        if(output.brtt==0){
            output.brtt=input.rtt;
        }
    }
    if(output.state==CongestionResponse){
        if(td.isEmpty()){
            output.state=Drain;
            output.halfdraincnt=0;
        }
    }
    if(output.state==DrainHalf){
        
    }
    if(output.state==Drain){
        
    }
    if(output.state==Growth){
        if(!td.isEmpty()){
            output.state=CongestionResponse;
            output.crtt=ipt.rtt;
            output.maxrate=output.rate;            
        }
    }
    
    //进行调速
    if(output.state==Startup){
        /*
         * 初始状态，需要快速试探出网络上限
         * R'=STARTUP_RATE_GAIN * R
         * STARTUP_RATE_GAIN 默认 2
         */
        output.rate=output.rate*STARTUP_RATE_GAIN;
    }
    if(output.state==CongestionResponse){
        /*
         * 对拥塞进行响应，计算最大可用速率
         */
        double cTime = input.elapsedTimes[td.last()] - input.elapsedTimes[td.first()];//单位ms
        output.fullrate=(output.rate*cTime - td.size()) / cTime;//排队满载计算公式
        output.rate=output.fullrate;
        //多次计算后，fullRate=rate<=真实网络瓶颈速度
    }
    if(output.state==Drain){
        /*
         * 排空队列到原来长度的50%
         */
        output.halfdraincnt++;
        output.rate=output.maxrate-output.fullrate;
    }
    if(output.state==Drain){
        /*
         * 排空队列到原来长度的
         */
        // output.rate = output.fullrate * DRAIN_RATE_DELTA;
    }
    
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
