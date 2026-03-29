#ifndef GENERAL_H
#define GENERAL_H

#pragma once

//调试宏定义
#include <QDebug>
#define noutput(method) q##method()<<"["<<#method<<']'<<__FILE__<<__func__<<__LINE__<<"\n"
#define ndb noutput(Debug)
#define ninfo noutput(Info)
#define nwarning noutput(Warning)
#define ncritical noutput(Critical)


//跨平台设备定义
#include <stdio.h>
#ifdef Q_OS_WIN
#define N_OS "Windows"
#define winComp(sth) sth
#define androidComp(sth)
#define linuxComp(sth)

#elif defined(Q_OS_ANDROID)
#define N_OS "Android"
#define winComp(sth) 
#define androidComp(sth) sth
#define linuxComp(sth)

#elif  defined(Q_OS_LINUX)
#define N_OS "Linux"
#define winComp(sth)
#define androidComp(sth)
#define linuxComp(sth) sth

#else
#define N_OS "Unknown"
#define winComp(sth)
#define androidComp(sth)
#define linuxComp(sth)
#endif

#define winRun if(strcmp(N_OS,"Windows")==0)
#define androidRun if(strcmp(N_OS,"Android")==0)
#define linuxRun if(strcmp(N_OS,"Linux")==0)


//类型简化定义
#include <modules/communication/communication.h>
typedef Communication::ipport ipport; 
typedef Communication::device device;


//工具函数定义
#include <QUrl>
#include <QMediaPlayer>
#include <QDir>
inline void playSound(QUrl url){
    QMediaPlayer* player = new QMediaPlayer();
    player->setMedia(QMediaContent(url));
    QObject::connect(player, &QMediaPlayer::stateChanged, player, [=](QMediaPlayer::State state){if(state==QMediaPlayer::StoppedState)player->deleteLater();});
    player->play();
}
inline bool operator<(QDir left,QDir right){
    return left.absolutePath() < right.absolutePath();
}

//安全报警
#ifndef QT_DEBUG
#ifdef DEBUG_NO_ENCRYPTION
#error "Safety Error:DEBUG_NO_ENCRYPTION"
#endif
#endif

#endif // GENERAL_H
