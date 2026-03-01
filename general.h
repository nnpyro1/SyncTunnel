#ifndef GENERAL_H
#define GENERAL_H

#pragma once

//调试宏定义
#include <QDebug>
#define noutput(method) q##method()<<'['<<#method<<']'<<__FILE__<<__func__<<__LINE__<<"\n"
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
#endif // GENERAL_H
