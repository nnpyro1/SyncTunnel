#include "mainwindow.h"
#include <QApplication>
#include <QSslSocket>
#include <QSplashScreen>
#include <QScreen>


int nMain(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    
    //注册
    qRegisterMetaType<QList<device>>();
    qRegisterMetaType<BusinessLogic::BusinessEvent>();
    qRegisterMetaType<TransmissionEngine::SendInfo>();
    
    QPixmap pixmap;pixmap.load(":/rc/img/splash1.png");
    pixmap=pixmap.scaledToHeight(QApplication::primaryScreen()->size().height()/2,Qt::SmoothTransformation);
    QSplashScreen splash(pixmap);
//    splash.setWindowFlags(/*Qt::WindowStaysOnTopHint | */Qt::SplashScreen);
//    splash.setAttribute(Qt::WA_DeleteOnClose);
    splash.show();
    for(int i=0;i<10;i++){splash.raise();
    a.processEvents();}
    
    //创建对象
    QThread workThread;
    BusinessLogic *bl = nullptr;
//    RUN_IN_CUSTOM_THREAD(&workThread,new BusinessLogic(nullptr););
    QObject *tmp = new QObject();
    tmp->moveToThread(&workThread);
    
//    bl->moveToThread(&workThread);
    workThread.start();
//    RUN_IN_CUSTOM_THREAD(&bl,bl.init());
    QMetaObject::invokeMethod(tmp,[&bl]{bl=new BusinessLogic;},Qt::BlockingQueuedConnection);
    QMetaObject::invokeMethod(bl,[=]{bl->init();},Qt::QueuedConnection);
    ViewModel *vm=new ViewModel(bl);
    
    MainWindow *w=new MainWindow(vm,nullptr,[&](QString a){splash.showMessage(a,Qt::AlignBottom|Qt::AlignHCenter);});
//    w->show();
//    splash.finish(w);
    {
        QObject obj;//用于自动销毁连接
        vm->o_status.use(&obj,[&splash,vm]{splash.showMessage(vm->o_status.get(),Qt::AlignHCenter|Qt::AlignBottom);});
        QEventLoop loop;
        QObject::connect(bl,&BusinessLogic::businessEventOccurred,&obj,[&loop](BusinessLogic::BusinessEvent event,[[maybe_unused]]QVariantMap m){
            if(event==BusinessLogic::BusinessEvent::LoadedSuccessfully){
                loop.quit();
            }
        });
        loop.exec();
    }
    w->show();
    splash.close();
    
    int result = a.exec();
    
    w->deleteLater();
    vm->deleteLater();
    tmp->deleteLater();
//    RUN_IN_CUSTOM_THREAD(bl,bl->destory(););
    QMetaObject::invokeMethod(bl,[=]{bl->destory();},Qt::BlockingQueuedConnection);
    bl->deleteLater();
    workThread.quit();
    workThread.wait(5000);
    if(!workThread.isFinished()){
        workThread.terminate();
    }
    return result;
}


int main(int argc,char *argv[]){
    do{
        int ret=nMain(argc,argv);
        if(ret!=EXIT_CODE_RESTART){
            return ret;
        }
    }while(1);
}