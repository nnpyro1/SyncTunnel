#include "congestioncontrol.h"


CongestionControl::CongestionControl(QObject *parent)
    : QObject{parent} {}

void CongestionControl::reset()
{
    
}

void CongestionControl::update(CongestionControlInput ipt){
    input=ipt;
}

CongestionControl::CongestionControlOutput CongestionControl::getOutput()
{
    return output;
}
