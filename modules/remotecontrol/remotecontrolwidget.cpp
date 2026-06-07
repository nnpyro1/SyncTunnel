#include "remotecontrolwidget.h"
#include "qcoreevent.h"
#include "qevent.h"
#include "qpainter.h"

RemoteControlWidget::RemoteControlWidget(QWidget *parent,RemoteControlEngine *eng)
    : QWidget{parent}
{
    setFocusPolicy(Qt::StrongFocus);
    installEventFilter(this);
    mouseMoveTimer.start(50);
    
    connect(&mouseMoveTimer,&QTimer::timeout,this,[this]{
        if(QCursor::pos() == lastMousePos){
            return;
        }
        
        if(state!=RemoteControlEngine::State::Controlling){
            return;
        }
        
        RemoteControlEngine::MouseMoveEvent me;//该事件仅允许0~1之间的小数
        QPointF p;
        p.setX( mapFromGlobal(QCursor::pos()).x()*1. / width() );
        p.setY( mapFromGlobal(QCursor::pos()).y()*1. / height() );
        me.pos=p;
        
        if(p.x()<0||p.x()>1||p.y()<0||p.y()>1){
            return;
        }
        
        RemoteControlEngine::RemoteEvent re;
        re.eventType=RemoteControlEngine::RemoteEventType::MouseMoveEvent;
        QDataStream stm(&re.data,QDataStream::ReadWrite);
        stm<<me;
        RUN_IN_CUSTOM_THREAD(this->eng,{
            this->eng->sendEvent(re);
        });
        lastMousePos=QCursor::pos();
    });
    
    setEngine(eng);
}

void RemoteControlWidget::setEngine(RemoteControlEngine *eng){
    this->eng=eng;
    connect(eng,&RemoteControlEngine::remoteScreenChanged,this,[this](QImage remoteScreen){this->remoteScreen=remoteScreen;update();});
    connect(eng,&RemoteControlEngine::stateChanged,this,[this](RemoteControlEngine::State state){
        this->state=state;
    });
}


void RemoteControlWidget::paintEvent([[maybe_unused]]QPaintEvent *event){
    QPainter painter(this);
    if(!eng){
        painter.drawText(0,0,width(),height(),Qt::AlignCenter,"<font color=\"red\">此远程控制组件无效</font><br>详细信息请咨询开发者");
        return;
    }
    if(state==RemoteControlEngine::State::Idle){
        painter.drawText(0,0,width(),height(),Qt::AlignCenter,"未开始远程控制");
    }
    else if(state==RemoteControlEngine::State::Controlled){
        painter.drawText(0,0,width(),height(),Qt::AlignCenter,"正在被远程控制 请勿操作");
    }
    else{
        painter.drawImage(QRect(0,0,width(),height()),remoteScreen);
    }
}


bool RemoteControlWidget::eventFilter(QObject *obj, QEvent *event){
    if(state!=RemoteControlEngine::State::Controlling){
        return QWidget::eventFilter(obj,event);
    }
    
    //鼠标类
    if(event->type() == QEvent::MouseButtonPress){
        RemoteControlEngine::MousePressEvent me;
        QMouseEvent *mouseEvent = (QMouseEvent*)event;
        me.btn=mouseEvent->button();
        me.pressed=true;
        
        RemoteControlEngine::RemoteEvent re;
        re.eventType=RemoteControlEngine::RemoteEventType::MousePressEvent;
        QDataStream stm(&re.data,QDataStream::ReadWrite);
        stm<<me;
        RUN_IN_CUSTOM_THREAD(eng,{
            eng->sendEvent(re);
        });
        return true;
    }
    if(event->type() == QEvent::MouseButtonRelease){
        RemoteControlEngine::MousePressEvent me;
        QMouseEvent *mouseEvent = (QMouseEvent*)event;
        me.btn=mouseEvent->button();
        me.pressed=false;
        
        RemoteControlEngine::RemoteEvent re;
        re.eventType=RemoteControlEngine::RemoteEventType::MousePressEvent;
        QDataStream stm(&re.data,QDataStream::ReadWrite);
        stm<<me;
        RUN_IN_CUSTOM_THREAD(eng,{
            eng->sendEvent(re);
        });
        return true;
    }
    //键盘类
    if(event->type() == QEvent::KeyPress){
        RemoteControlEngine::KeyEvent ke;
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        ke.key=(Qt::Key)keyEvent->key();
        ke.pressed=true;
        
        RemoteControlEngine::RemoteEvent re;
        re.eventType=RemoteControlEngine::RemoteEventType::KeyEvent;
        QDataStream stm(&re.data,QDataStream::ReadWrite);
        stm<<ke;
        RUN_IN_CUSTOM_THREAD(eng,{
            eng->sendEvent(re);
        });
        return true;
    }
    if(event->type() == QEvent::KeyRelease){
        RemoteControlEngine::KeyEvent ke;
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        ke.key=(Qt::Key)keyEvent->key();
        ke.pressed=false;
        
        RemoteControlEngine::RemoteEvent re;
        re.eventType=RemoteControlEngine::RemoteEventType::KeyEvent;
        QDataStream stm(&re.data,QDataStream::ReadWrite);
        stm<<ke;
        RUN_IN_CUSTOM_THREAD(eng,{
            eng->sendEvent(re);
        });
        return true;
    }
    
    return QWidget::eventFilter(obj,event);
}
