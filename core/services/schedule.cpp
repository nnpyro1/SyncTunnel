#include "schedule.h"

Schedule::Schedule(QObject *parent) : QObject(parent){
    
}


void Schedule::setType(Schedule::Type t){
    this->tp=t;
}

void Schedule::setRepeat(Schedule::Repeat rep){
    this->rep=rep;
}

void Schedule::setTime(QTime t){
    this->tm=t;
}

void Schedule::setDateTime(QDateTime t){
    this->dtm=t;
}


void Schedule::set(QDateTime t){
    tp = t_datetime;
    dtm = t;
}

void Schedule::set(QTime t, Schedule::Repeat repeat){
    tp = t_time;
    tm = t;
    rep = repeat;
}

void Schedule::set(long sec){
    set(QDateTime::currentDateTime().addSecs(sec));
}


void Schedule::setEnabled(bool enabled){
    this->enabled = enabled;
}

bool Schedule::isEnabled(){
    return enabled;
}

void Schedule::setAutoTrigger(bool autoTrigger){
    if(autoTrigger){
        if(!timer){
            timer = new QTimer;
        }
        connect(timer,&QTimer::timeout,this,&Schedule::checkTrigger);
        timer->start(interval());
    }
    else{
        timer->deleteLater();
        timer = nullptr;
    }
}

bool Schedule::autoTrigger(){
    return timer!=nullptr;
}

void Schedule::setInterval(int it){
    interval_ = it;
}

int Schedule::interval(){
    return interval_;
}


bool Schedule::checkTrigger(){
    //检查
    if(!enabled){
        return false;
    }
    if(tp == t_datetime){
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QDate stDate = dtm.date();
        QTime stTime = dtm.time();
        
        //粗侦测
        if(stDate != currentDateTime.date()){//日期不同
            return false;
        }
        if(stTime.hour() != currentDateTime.time().hour()){//小时不同
            return false;
        }
        if(stTime > currentDateTime.time()){//设置的时间更晚，一定没到
            return false;
        }
        
        //进行精细侦测
        if(autoTrigger()){
            //自动触发，允许偏差是定时器精度的2倍-1
            long difference = stTime.msecsTo(currentDateTime.time());
            if(difference < 2*interval()-1){
                setEnabled(false);//等一会防止多次触发
                QTimer::singleShot(2*interval()+2,this,[this]{setEnabled(true);});
                emit triggered();
                return true;
            }
        }
        else{
            long difference = stTime.msecsTo(currentDateTime.time());
            if(difference < 60){
                setEnabled(false);//等一会防止多次触发
                QTimer::singleShot(2*60+2,this,[this]{setEnabled(true);});
                emit triggered();
                return true;
            }
        }
    }
    
    else if(tp == t_time){
        //侦测星期
        QDateTime currentDateTime = QDateTime::currentDateTime();
//        QTime currentTime = QTime::currentTime();
        Repeat currentDayOfWeek;
        switch(currentDateTime.date().dayOfWeek()){
        case Qt::Monday:
            currentDayOfWeek = rp_Monday;
            break;
        case Qt::Tuesday:
            currentDayOfWeek = rp_Tuesday;
            break;
        case Qt::Wednesday:
            currentDayOfWeek = rp_Wednesday;
            break;
        case Qt::Thursday:
            currentDayOfWeek = rp_Thursday;
            break;
        case Qt::Friday:
            currentDayOfWeek = rp_Friday;
            break;
        case Qt::Saturday:
            currentDayOfWeek = rp_Saturday;
            break;
        case Qt::Sunday:
            currentDayOfWeek = rp_Sunday;
            break;
        }
        
        //侦测日期
        if((repeat() & currentDayOfWeek) == 0){
            return false;
        }
        
        //粗侦测
        if(time().hour() != currentDateTime.time().hour()){//小时不同
            return false;
        }
        if(time() > currentDateTime.time()){//设置的时间更晚，一定没到
            return false;
        }
        
        //进行精细侦测
        if(autoTrigger()){
            //自动触发，允许偏差是定时器精度的2倍-1
            long difference = time().msecsTo(currentDateTime.time());
            if(difference < 2*interval()-1){
                setEnabled(false);//等一会防止多次触发
                QTimer::singleShot(2*interval()+2,this,[this]{setEnabled(true);});
                emit triggered();
                return true;
            }
        }
        else{
            long difference = time().msecsTo(currentDateTime.time());
            if(difference < 60){
                setEnabled(false);//等一会防止多次触发
                QTimer::singleShot(2*60+2,this,[this]{setEnabled(true);});
                emit triggered();
                return true;
            }
        }
    }

    
    return false;
}


QString Schedule::toString() {
    QStringList parts;
    
    // 添加基本信息
    parts << QString("%1").arg(enabled ? "" : "*");
    
    
    // 根据类型添加详细信息
    if (tp == t_datetime) {
        parts << "单次";
        parts << QString("%1触发").arg(dtm.toString("yyyy-MM-dd hh:mm:ss"));
    } 
    else if (tp == t_time) {
        parts << "重复";
        parts << QString("%1触发").arg(tm.toString("hh:mm:ss"));
        
        // 处理重复设置
        QStringList repeatDays;
        if (rep == 0) {
            repeatDays << "None";
        } else {
            if (rep & rp_Monday)    repeatDays << "周一";
            if (rep & rp_Tuesday)   repeatDays << "周二";
            if (rep & rp_Wednesday) repeatDays << "周三";
            if (rep & rp_Thursday)  repeatDays << "周四";
            if (rep & rp_Friday)    repeatDays << "周五";
            if (rep & rp_Saturday)  repeatDays << "周六";
            if (rep & rp_Sunday)    repeatDays << "周日";
        }
        
        parts << QString("重复:%1").arg(repeatDays.join(" "));
    }
    else {
        parts << "Type: Invalid";
    }
    
    return parts.join(" ");
}


QDataStream &operator<<(QDataStream &stream,const Schedule &s){
    return stream<<s.tp<<s.rep<<s.tm<<s.dtm<<(s.timer!=nullptr)<<s.enabled<<s.interval_;
}

QDataStream &operator>>(QDataStream &stream,Schedule &s){
    bool use_timer;
    stream>>s.tp>>s.rep>>s.tm>>s.dtm>>use_timer>>s.enabled>>s.interval_;
    if(use_timer){
        if(!s.timer){
            s.timer=new QTimer;
            s.connect(s.timer, &QTimer::timeout, &s, &Schedule::checkTrigger);
        }
        s.timer->start(s.interval_);
    }
    return stream;
}
