#include "mainwindow.h"
#include <QApplication>
#include <QSslSocket>
#include <QSplashScreen>

MainWindow* g_mainWindow = nullptr;

void resetMainWindow()
{
    QMetaObject::invokeMethod(qApp,[]{
        if (g_mainWindow)
        {
            g_mainWindow->hide();
            g_mainWindow->deleteLater();
            g_mainWindow = nullptr;
        }
        
        g_mainWindow = new MainWindow(nullptr,nullptr,true);
        g_mainWindow->show();
    },Qt::QueuedConnection);
}

int main(int argc, char *argv[])
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
    
    g_mainWindow = new MainWindow(nullptr,[&](QString a){splash.showMessage(a,Qt::AlignBottom|Qt::AlignHCenter);});
    g_mainWindow->show();
    splash.finish(g_mainWindow);
    return a.exec();
}