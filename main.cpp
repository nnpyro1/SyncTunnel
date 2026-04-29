#include "mainwindow.h"
#include <QApplication>
#include <QSslSocket>
#include <QSplashScreen>


int nMain(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    
    QPixmap pixmap;pixmap.load(":/rc/img/splash1.png");
    pixmap=pixmap.scaledToHeight(QApplication::primaryScreen()->size().height()/2,Qt::SmoothTransformation);
    QSplashScreen splash(pixmap);
//    splash.setWindowFlags(/*Qt::WindowStaysOnTopHint | */Qt::SplashScreen);
    
    splash.show();
    for(int i=0;i<10;i++){splash.raise();
    a.processEvents();}
    
    //创建对象
    QThread workThread;
    BusinessLogic bl(nullptr);
    bl.moveToThread(&workThread);
    workThread.start();
    
    MainWindow w(&bl,nullptr,[&](QString a){splash.showMessage(a,Qt::AlignBottom|Qt::AlignHCenter);});
    w.show();
    splash.finish(&w);
    return a.exec();
}


int main(int argc,char *argv[]){
    do{
        int ret=nMain(argc,argv);
        if(ret!=EXIT_CODE_RESTART){
            return ret;
        }
    }while(1);
}