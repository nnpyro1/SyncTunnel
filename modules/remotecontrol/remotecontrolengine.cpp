#include "remotecontrolengine.h"
#include "qbuffer.h"
#include "qcursor.h"
#include "qmetaobject.h"
#include "qpainter.h"
#include "qscreen.h"
#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QColorSpace>
#include <qbuffer.h>

RemoteControlEngine::RemoteControlEngine(TransmissionEngine *te, QObject *parent)
    : QObject{parent} ,
    te(te)
{
    if(this->thread()!=te->thread()){
        ncritical<<"RemoteControlEngine:TransmissionEngine线程不在同一线程";
    }
    
    connect(this->te,&TransmissionEngine::SPTP_commonMsgReceived,this,[this](TransmissionEngine::msg_common msg){
        if(msg.msg.startsWith("__RMCT__")){
            parseEvent(msg.msg);
        }
    });
    connect(this,&RemoteControlEngine::eventReceived,this,[this](RemoteEvent e){RUN_IN_MAIN_THREAD(this->handleReceivedEvent(e););});
    connect(this->te,&TransmissionEngine::reliableMessageReceived,this,[this](QString msg,int sender){
        if(!msg.startsWith("__RMCT_R__")) return;
        msg=msg.mid(10);
        if(msg=="START_CONTROL"){
            currentState=State::Controlled;//被控
            refreshTimer.start();
            currentId = sender;
            emit stateChanged(currentState);
            //计算分割
            QSize screenSize;
            QMetaObject::invokeMethod(qApp,[&screenSize]{screenSize=qApp->primaryScreen()->size();},Qt::BlockingQueuedConnection);
            chunkSize=calculateChunkSize(screenSize,maxChunkArea);
            currentScreenChunkIndex=0;
            ninfo<<"开始被"<<currentId<<"远控 屏幕分块大小"<<chunkSize;
        }
        else if(msg=="FINISH_CONTROL"){
            currentState=State::Idle;
            currentId = -1;
            refreshTimer.stop();
            emit stateChanged(currentState);
        }
    });
    connect(&refreshTimer,&QTimer::timeout,this,&RemoteControlEngine::sendScreen);
    
    refreshTimer.setSingleShot(false);
    refreshTimer.setInterval(5);
    
    //生成键盘映射关系
    {
        keyMap.insert(Qt::Key_Left,0x25);
        keyMap.insert(Qt::Key_Up,0x26);
        keyMap.insert(Qt::Key_Right,0x27);
        keyMap.insert(Qt::Key_Down,0x28);
        keyMap.insert(Qt::Key_Backspace,0x08);
        keyMap.insert(Qt::Key_Tab,0x09);
        keyMap.insert(Qt::Key_Clear,0x0C);
        keyMap.insert(Qt::Key_Return,0x0D);
        keyMap.insert(Qt::Key_Enter,0x0D);
        keyMap.insert(Qt::Key_Shift,0x10);
        keyMap.insert(Qt::Key_Control,0x11);
        keyMap.insert(Qt::Key_Alt,0x12);
        keyMap.insert(Qt::Key_Pause,0x13);
        keyMap.insert(Qt::Key_CapsLock,0x14);
        keyMap.insert(Qt::Key_Escape,0x1B);
        keyMap.insert(Qt::Key_Space,0x20);
        keyMap.insert(Qt::Key_PageUp,0x21);
        keyMap.insert(Qt::Key_PageDown,0x22);
        keyMap.insert(Qt::Key_End,0x23);
        keyMap.insert(Qt::Key_Home,0x24);
        keyMap.insert(Qt::Key_Select,0x29);
        keyMap.insert(Qt::Key_Print,0x2A);
        keyMap.insert(Qt::Key_Execute,0x2B);
        keyMap.insert(Qt::Key_Printer,0x2C);
        keyMap.insert(Qt::Key_Insert,0x2D);
        keyMap.insert(Qt::Key_Delete,0x2E);
        keyMap.insert(Qt::Key_Help,0x2F);
        keyMap.insert(Qt::Key_0,0x30);
        keyMap.insert(Qt::Key_ParenRight,0x30); // )
        keyMap.insert(Qt::Key_1,0x31);
        keyMap.insert(Qt::Key_Exclam,0x31); // !
        keyMap.insert(Qt::Key_2,0x32);
        keyMap.insert(Qt::Key_At,0x32); // @
        keyMap.insert(Qt::Key_3,0x33);
        keyMap.insert(Qt::Key_NumberSign,0x33); // #
        keyMap.insert(Qt::Key_4,0x34);
        keyMap.insert(Qt::Key_Dollar,0x34); // $
        keyMap.insert(Qt::Key_5,0x35);
        keyMap.insert(Qt::Key_Percent,0x35); // %
        keyMap.insert(Qt::Key_6,0x36);
        keyMap.insert(Qt::Key_AsciiCircum,0x36); // ^
        keyMap.insert(Qt::Key_7,0x37);
        keyMap.insert(Qt::Key_Ampersand,0x37); // &
        keyMap.insert(Qt::Key_8,0x38);
        keyMap.insert(Qt::Key_Asterisk,0x38); // *
        keyMap.insert(Qt::Key_9,0x39);
        keyMap.insert(Qt::Key_ParenLeft,0x39); // (
        keyMap.insert(Qt::Key_A,0x41);
        keyMap.insert(Qt::Key_B,0x42);
        keyMap.insert(Qt::Key_C,0x43);
        keyMap.insert(Qt::Key_D,0x44);
        keyMap.insert(Qt::Key_E,0x45);
        keyMap.insert(Qt::Key_F,0x46);
        keyMap.insert(Qt::Key_G,0x47);
        keyMap.insert(Qt::Key_H,0x48);
        keyMap.insert(Qt::Key_I,0x49);
        keyMap.insert(Qt::Key_J,0x4A);
        keyMap.insert(Qt::Key_K,0x4B);
        keyMap.insert(Qt::Key_L,0x4C);
        keyMap.insert(Qt::Key_M,0x4D);
        keyMap.insert(Qt::Key_N,0x4E);
        keyMap.insert(Qt::Key_O,0x4F);
        keyMap.insert(Qt::Key_P,0x50);
        keyMap.insert(Qt::Key_Q,0x51);
        keyMap.insert(Qt::Key_R,0x52);
        keyMap.insert(Qt::Key_S,0x53);
        keyMap.insert(Qt::Key_T,0x54);
        keyMap.insert(Qt::Key_U,0x55);
        keyMap.insert(Qt::Key_V,0x56);
        keyMap.insert(Qt::Key_W,0x57);
        keyMap.insert(Qt::Key_X,0x58);
        keyMap.insert(Qt::Key_Y,0x59);
        keyMap.insert(Qt::Key_Z,0x5A);
        keyMap.insert(Qt::Key_multiply,0x6A);
        keyMap.insert(Qt::Key_F1,0x70);
        keyMap.insert(Qt::Key_F2,0x71);
        keyMap.insert(Qt::Key_F3,0x72);
        keyMap.insert(Qt::Key_F4,0x73);
        keyMap.insert(Qt::Key_F5,0x74);
        keyMap.insert(Qt::Key_F6,0x75);
        keyMap.insert(Qt::Key_F7,0x76);
        keyMap.insert(Qt::Key_F8,0x77);
        keyMap.insert(Qt::Key_F9,0x78);
        keyMap.insert(Qt::Key_F10,0x79);
        keyMap.insert(Qt::Key_F11,0x7A);
        keyMap.insert(Qt::Key_F12,0x7B);
        keyMap.insert(Qt::Key_F13,0x7C);
        keyMap.insert(Qt::Key_F14,0x7D);
        keyMap.insert(Qt::Key_F15,0x7E);
        keyMap.insert(Qt::Key_F16,0x7F);
        keyMap.insert(Qt::Key_F17,0x80);
        keyMap.insert(Qt::Key_F18,0x81);
        keyMap.insert(Qt::Key_F19,0x82);
        keyMap.insert(Qt::Key_F20,0x83);
        keyMap.insert(Qt::Key_F21,0x84);
        keyMap.insert(Qt::Key_F22,0x85);
        keyMap.insert(Qt::Key_F23,0x86);
        keyMap.insert(Qt::Key_F24,0x87);
        keyMap.insert(Qt::Key_NumLock,0x90);
        keyMap.insert(Qt::Key_ScrollLock,0x91);
        keyMap.insert(Qt::Key_VolumeDown,0xAE);
        keyMap.insert(Qt::Key_VolumeUp,0xAF);
        keyMap.insert(Qt::Key_VolumeMute,0xAD);
        keyMap.insert(Qt::Key_MediaStop,0xB2);
        keyMap.insert(Qt::Key_MediaPlay,0xB3);
        keyMap.insert(Qt::Key_Plus,0xBB); // +
        keyMap.insert(Qt::Key_Minus,0xBD); // -
        keyMap.insert(Qt::Key_Underscore,0xBD); // _
        keyMap.insert(Qt::Key_Equal,0xBB); // =
        keyMap.insert(Qt::Key_Semicolon,0xBA); // ;
        keyMap.insert(Qt::Key_Colon,0xBA); // :
        keyMap.insert(Qt::Key_Comma,0xBC); // ,
        keyMap.insert(Qt::Key_Less,0xBC); // <
        keyMap.insert(Qt::Key_Period,0xBE); // .
        keyMap.insert(Qt::Key_Greater,0xBE); // >
        keyMap.insert(Qt::Key_Slash,0xBF);  // /
        keyMap.insert(Qt::Key_Question,0xBF); // ?
        keyMap.insert(Qt::Key_BracketLeft,0xDB); // [
        keyMap.insert(Qt::Key_BraceLeft,0xDB); // {
        keyMap.insert(Qt::Key_BracketRight,0xDD); // ]
        keyMap.insert(Qt::Key_BraceRight,0xDD); // }
        keyMap.insert(Qt::Key_Bar,0xDC); // |
        keyMap.insert(Qt::Key_Backslash,0xDC); // 反斜杠
        keyMap.insert(Qt::Key_Apostrophe,0xDE); // '
        keyMap.insert(Qt::Key_QuoteDbl,0xDE); // "
        keyMap.insert(Qt::Key_QuoteLeft,0xC0); // `
        keyMap.insert(Qt::Key_AsciiTilde,0xC0); // ~
    }
}


bool RemoteControlEngine::startControl(int id){
    if(currentState!=State::Idle){
        nwarning<<"无法在非空闲状态开始远程控制 currentState ="<<QMetaEnum::fromType<RemoteControlEngine::State>().valueToKey((int)currentState);
        return false;
    }
    //发送开始控制消息
    bool succeeded = false;
    for(int i=0;i<3;i++){
        if(te->sendReliableMessage(id,"__RMCT_R__START_CONTROL")){
            succeeded = true;
            break;
        }
    }
    if(!succeeded){
        nwarning<<"无法开始远控";
        return false;
    }
    
    //开始控制
    currentId=id;
    currentState=State::Controlling;
    emit stateChanged(currentState);
    return true;
}

bool RemoteControlEngine::stopControl(){
    if(currentId<0){
        nwarning<<"RemoteControlEngine: currentId无效 无法开始远控";
        return false;
    }
    
    //发送停止消息
    bool succeeded = false;
    for(int i=0;i<5;i++){
        if(te->sendReliableMessage(currentId,"__RMCT_R__FINISH_CONTROL")){
            succeeded = true;
            break;
        }
    }
    if(!succeeded){
        nwarning<<"无法停止远控";
        return false;
    }
    currentId=-1;
    currentState=State::Idle;
    emit stateChanged(currentState);
    return true;
}

bool RemoteControlEngine::sendEvent(RemoteEvent event){
    if(currentState==State::Idle || currentId<0){
        nwarning<<"RemoteControlEngine: Unable to send event: currentState==State::Idle || currentId<0";
        ninfo<<"state:"<<QMetaEnum::fromType<State>().valueToKey((int)currentState)<<"currentId"<<currentId;
        return false;
    }
    QByteArray a;
    QDataStream s(&a,QDataStream::ReadWrite);s<<event;
    te->SPTP_sendCommon("__RMCT__"+a,currentId);
    return true;
}


void RemoteControlEngine::handleReceivedEvent(RemoteEvent event){
    QDataStream stm(event.data);
    
    switch(event.eventType){
    case RemoteEventType::MouseMoveEvent:{
        MouseMoveEvent e;
        stm>>e;
        QCursor::setPos(e.pos.x()*qApp->primaryScreen()->geometry().width(),e.pos.y()*qApp->primaryScreen()->geometry().height());
        
        break;
    }
        
    case RemoteEventType::MousePressEvent:{
        MousePressEvent e;
        stm>>e;
        winComp(
            INPUT ipt={};
            ipt.type=INPUT_MOUSE;
            
            int flg=0;
            switch(e.btn){  
            case Qt::LeftButton:
                if(e.pressed){
                    flg=MOUSEEVENTF_LEFTDOWN;
                }
                else{
                    flg=MOUSEEVENTF_LEFTUP;
                }
                break;
            
            case Qt::MiddleButton:
                if(e.pressed){
                    flg=MOUSEEVENTF_MIDDLEDOWN;
                }
                else{
                    flg=MOUSEEVENTF_MIDDLEUP;
                }
                break;
            
            case Qt::RightButton:
                if(e.pressed){
                    flg=MOUSEEVENTF_RIGHTDOWN;
                }
                else{
                    flg=MOUSEEVENTF_RIGHTUP;                    
                }
                break;
            default:
                break;
            }
            if(flg!=0){
                ipt.mi.dwFlags=flg;
                SendInput(1,&ipt,sizeof(ipt));
            }
        )
        break;
    }
        
    case RemoteEventType::KeyEvent:{
        KeyEvent e;
        stm>>e;
        winComp({
            INPUT ipt={};
            ipt.type=INPUT_KEYBOARD;
            ipt.ki.wVk=keyMap[e.key];//转换到VK
            ipt.ki.dwFlags=e.pressed?0:KEYEVENTF_KEYUP;
            SendInput(1,&ipt,sizeof(ipt));
        })
        break;
    }
    case RemoteEventType::ControlEvent:
        //暂时废弃
        break;
    case RemoteEventType::ScreenEvent:
        handleScreen(event.data);
        break;
    }
}


void RemoteControlEngine::sendScreen(){
    //计算截取坐标
    QSize screenSize;
    QMetaObject::invokeMethod(qApp,[&screenSize]{screenSize=qApp->primaryScreen()->size();},Qt::BlockingQueuedConnection);
    int maxHChunks = screenSize.width()/chunkSize.width();
    int maxVChunks = screenSize.height()/chunkSize.height();
    int startX = (currentScreenChunkIndex%maxHChunks)*chunkSize.width();
    int startY = (currentScreenChunkIndex/maxHChunks)*chunkSize.height();
    //截屏
    QImage screen;
    QMetaObject::invokeMethod(qApp,[&screen]{screen=qApp->primaryScreen()->grabWindow().toImage();},Qt::BlockingQueuedConnection);
    //截取
    QImage img;
    img=screen.copy(startX,startY,chunkSize.width(),chunkSize.height()).convertToFormat(QImage::Format_RGB16);
    //压缩
    QByteArray imageData;
    QBuffer buf(&imageData);
    img.save(&buf,"WEBP",imageQuality);
    buf.close();
    
    //构造包
    RemoteScreen datagram;
    datagram.screenSize=screenSize;
    datagram.imageArea=QRect(startX,startY,chunkSize.width(),chunkSize.height());
    datagram.img=imageData;
    //构造事件
    QByteArray screenData;
    QDataStream stm(&screenData,QDataStream::ReadWrite);
    stm<<datagram;
    RemoteEvent event;
    event.eventType=RemoteEventType::ScreenEvent;
    event.data=screenData;
    //发送
    sendEvent(event);
    //更新索引
    currentScreenChunkIndex++;
    if(currentScreenChunkIndex>=maxHChunks*maxVChunks){
        currentScreenChunkIndex=0;
    }
}


void RemoteControlEngine::handleScreen(QByteArray data){
    RemoteScreen datagram;
    QDataStream stm(data); stm>>datagram;
    //强制缩放原来的图片
    QImage tempImg(datagram.screenSize.width(),datagram.screenSize.height(),QImage::Format_RGB16);
    tempImg.fill(Qt::black);
    QPainter pt(&tempImg);
    pt.drawImage(0,0,remoteScreen);
    //修改数据块
    QBuffer buf(&datagram.img);
    buf.open(QBuffer::ReadWrite);
    QImage chunk;chunk.load(&buf,"WEBP");
    pt.drawImage(datagram.imageArea,chunk);
    pt.end();
    remoteScreen=tempImg;
    //触发信号
    emit remoteScreenChanged(remoteScreen);
    buf.close();
}


QList<int> RemoteControlEngine::getDivisors(int num){
    QList<int> ret;
    for(int i=1;i<=num;i++){
        if(num%i==0){
            ret.append(i);
        }
    }
    return ret;
}


QSize RemoteControlEngine::calculateChunkSize(QSize screenSize, int maxArea){
    QList<int> divX=getDivisors(screenSize.width()),
        divY=getDivisors(screenSize.height());//获取所有因数
    //查找所有可能解
    QList<QPair<int,int>> solutions;//分的块数
    for(auto x=divX.rbegin();x!=divX.rend();x++){
        for(auto y=divY.rbegin();y!=divY.rend();y++){
            int w=screenSize.width() / *x;
            int h=screenSize.height() / *y;
            if((qint64)w*h <= maxArea){
                solutions.append(qMakePair(*x,*y));
            }
        }
    }

    //查找最优解
    int minChunks=INT_MAX;
    QSize bestSolution;
    for(auto i : solutions){
        if(i.first * i.second < minChunks){
            minChunks=i.first*i.second;
            bestSolution=QSize(screenSize.width()/i.first,screenSize.height()/i.second);
        }
    }
    
    return bestSolution;
}


RemoteControlEngine::RemoteEvent RemoteControlEngine::parseEvent(QByteArray msg){
    QByteArray eventMessage = msg.mid(8);
    QDataStream stream(eventMessage);
    RemoteEvent re;
    stream>>re;
    emit eventReceived(re);
    return re;
}
