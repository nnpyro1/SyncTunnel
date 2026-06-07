#ifndef REMOTECONTROLWIDGET_H
#define REMOTECONTROLWIDGET_H

#include <QWidget>
#include "remotecontrolengine.h"

class RemoteControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RemoteControlWidget(QWidget *parent = nullptr,RemoteControlEngine *eng=nullptr);
    
    void setEngine(RemoteControlEngine *eng);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj,QEvent *event) override;
private:
    
private:
    RemoteControlEngine *eng;
    QTimer mouseMoveTimer;
    QPoint lastMousePos;
    QImage remoteScreen;
    RemoteControlEngine::State state;
};

#endif // REMOTECONTROLWIDGET_H
