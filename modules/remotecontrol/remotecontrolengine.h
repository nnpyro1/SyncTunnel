#ifndef REMOTECONTROLENGINE_H
#define REMOTECONTROLENGINE_H


#include "qimage.h"
#include "qpaintdevice.h"
#include "qpixmap.h"
#include "qpoint.h"
#include "qsize.h"
#include <QObject>
#include <modules/transmissionengine/transmissionengine.h>
#include <general.h>
#include <utils.h>


class RemoteControlEngine : public QObject
{
    Q_OBJECT
public:
    enum class State{
        Idle,
        Controlling,
        Controlled,
    };
    Q_ENUM(State);
    enum class RemoteEventType{
        MouseMoveEvent,
        MousePressEvent,
        KeyEvent,
        ControlEvent,
        ScreenEvent,
    };
    struct MouseMoveEvent{
        QPointF pos;
        
        friend QDataStream &operator<<(QDataStream &s, const MouseMoveEvent &d) {
            return s << d.pos;
        }
        friend QDataStream &operator>>(QDataStream &s, MouseMoveEvent &d) {
            s >> d.pos;
            return s;
        }
    };
    struct MousePressEvent{
        Qt::MouseButton btn;
        bool pressed;
        
        friend QDataStream &operator<<(QDataStream &s, const MousePressEvent &d) {
            return s << (int)d.btn << (int)d.pressed;
        }
        friend QDataStream &operator>>(QDataStream &s, MousePressEvent &d) {
            int b;s >> b >> d.pressed;
            d.btn=(Qt::MouseButton)b;
            return s;
        }
    };

    struct KeyEvent{
        Qt::Key key;
        bool pressed;
        friend QDataStream &operator<<(QDataStream &s, const KeyEvent &d) {
            return s << ((int)d.key) << d.pressed;
        }
        friend QDataStream &operator>>(QDataStream &s, KeyEvent &d) {
            int k;
            s >> k >> d.pressed;
            d.key=Qt::Key(k);
            return s;
        }
    };
    
    struct ControlEvent{
        QString msg;
        
        friend QDataStream &operator<<(QDataStream &s, const ControlEvent &d) {
            return s << d.msg;
        }
        friend QDataStream &operator>>(QDataStream &s, ControlEvent &d) {
            s >> d.msg;
            return s;
        }
    };

    struct RemoteEvent{
        RemoteControlEngine::RemoteEventType eventType;
        QByteArray data;
        
        friend QDataStream &operator<<(QDataStream &s, const RemoteEvent &d) {
            s << static_cast<int>(d.eventType) << d.data;
            return s;
        }
        
        friend QDataStream &operator>>(QDataStream &s, RemoteEvent &d) {
            int type;
            s >> type >> d.data;
            d.eventType = static_cast<RemoteEventType>(type);
            return s;
        }
    };

public:
    explicit RemoteControlEngine(TransmissionEngine *te,QObject *parent = nullptr);
    
public slots:
    bool startControl(int id);
    bool stopControl();
    bool sendEvent(RemoteControlEngine::RemoteEvent event);
    RemoteControlEngine::State state(){return currentState;}
signals:
    void eventReceived(RemoteControlEngine::RemoteEvent event);//无需处理，已经内部做过了
    void stateChanged(RemoteControlEngine::State state);
    void remoteScreenChanged(QImage screen);
private:
    struct RemoteScreen{
        QSize screenSize;
        QRect imageArea;
        QByteArray img;
        
        friend QDataStream &operator<<(QDataStream &s, const RemoteScreen &d) {
            return s << d.screenSize << d.imageArea << d.img;
        }
        friend QDataStream &operator>>(QDataStream &s, RemoteScreen &d) {
            s >> d.screenSize >> d.imageArea >> d.img;
            return s;
        }
    };
    RemoteEvent parseEvent(QByteArray msg);
    void handleReceivedEvent(RemoteControlEngine::RemoteEvent event);
    void sendScreen();
    void handleScreen(QByteArray imgData);
    QList<int> getDivisors(int num); 
    QSize calculateChunkSize(QSize screenSize,int maxArea);
private:
    TransmissionEngine *te;
    QTimer refreshTimer;
    int currentId=-1;
    State currentState = State::Idle;
    QMap<int,char> keyMap;//键盘映射表
    QSize chunkSize;
    int maxChunkArea = 18000;
    int currentScreenChunkIndex = 0;
    int imageQuality = 30;
    QImage remoteScreen;
};

// Q_DECLARE_METATYPE(RemoteControlEngine::State);
Q_DECLARE_METATYPE(RemoteControlEngine::RemoteEventType);
Q_DECLARE_METATYPE(RemoteControlEngine::RemoteEvent);

#endif // REMOTECONTROLENGINE_H
