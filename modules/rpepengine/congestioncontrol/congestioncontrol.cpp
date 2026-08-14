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
        }
        break;
    }
    case Growth:{
        if(!tl.empty()){
            output.state=CongestionResponse;
        }
        break;
    }
    case CongestionResponse:{
        output.state=ProbeMaxRate1;
        output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
        break;
    }
    case ProbeMaxRate1:{
        if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
            output.state=ProbeMaxRate2;
            output.probeTimeout=timer.elapsed()+PROBE_TIMEOUT;
        }
        break;
    }
    case ProbeMaxRate2:{
        if(!tl.empty() || timer.elapsed()>=output.probeTimeout){
            output.state=Drain;
            output.lastRttReportEnd=input.chunkId;
        }
        break;
    }
    case Drain:{
        if(input.end>=(quint32)output.lastRttReportEnd){//一个RTT过去了
            output.state=Growth;
        }
        break;
    }
    }
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
