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


//工具宏定义
#define RUN_LATER(sth) QMetaObject::invokeMethod(this,[=]{sth},Qt::QueuedConnection)
#define RUN_IN_MAIN_THREAD(sth) QMetaObject::invokeMethod(qApp,[=]{sth},Qt::QueuedConnection)
#define RUN_IN_CUSTOM_THREAD(obj,sth) QMetaObject::invokeMethod((obj),[=]{sth},Qt::QueuedConnection)
#define RUN_IN_CUSTOM_THREAD_THIS(obj,sth) QMetaObject::invokeMethod((obj),[=,this]{sth},Qt::QueuedConnection)


//仅值宏
#define EXIT_CODE_RESTART  ( 'R' << 16 | 'S' << 8 | 'T' )


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
#include <QAudioOutput>
inline void playSound(QUrl url){
    QMediaPlayer* player = new QMediaPlayer();
    QAudioOutput *output = new QAudioOutput;
    player->setSource((url));
    QObject::connect(player, &QMediaPlayer::playbackStateChanged, player, [=](QMediaPlayer::PlaybackState state){if(state==QMediaPlayer::StoppedState){player->deleteLater();output->deleteLater();}});
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


//自定义类型
struct Result{
    bool is_succeeded;
    QString errorMessage;
    
    Result():is_succeeded(true),errorMessage(QString()){}
    Result(QString errorMessage):is_succeeded(false),errorMessage(errorMessage){}
    operator bool (){return is_succeeded;}
};

#endif // GENERAL_H
