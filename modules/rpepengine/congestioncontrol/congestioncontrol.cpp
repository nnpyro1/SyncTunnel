#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {}

void CongestionControl::reset()
{
    
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
    
    //算法
    output.rate=40;
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
