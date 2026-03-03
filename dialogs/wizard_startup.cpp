#include "wizard_startup.h"
#include "ui_wizard_startup.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QTimer>

Wizard_startup::Wizard_startup(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::Wizard_startup)
{
    ui->setupUi(this);
    connect(this,&Wizard_startup::accepted,this,[this]{
        emit settingsSaved(ui->lineEdit_userName->text(),ui->lineEdit_password->text(),ui->lineEdit_github->text(),ui->lineEdit_github_pat->text());
    });
}

Wizard_startup::~Wizard_startup()
{
    delete ui;
}

bool Wizard_startup::validateCurrentPage(){
    if(currentPage()->property("pageType")=="passport"){
        QString user_name=ui->lineEdit_userName->text();
        QString pwd = ui->lineEdit_password->text();
        if(user_name.size()<8||pwd.size()<8){
            currentPage()->setSubTitle("<font color=\"red\">用户名或密码过短</font>");
            return false;
        }
        return true;
    }
    if(currentPage()->property("pageType")=="github"&&ui->radioButton_github_default->isChecked()){
        currentPage()->setSubTitle("<font color=\"#FF00FF\">正在获取默认用户名，请耐心等候......</font>");
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,80);
        
        //网络请求
        QNetworkAccessManager *manager = new QNetworkAccessManager;
        QNetworkRequest request;
        request.setUrl(QUrl("https://nnpyro.netlify.app/synctunnel-interface/github_pat.txt"));
        request.setHeader(QNetworkRequest::UserAgentHeader,"nnpyro SyncTunnel/vbeta-Unknown");
        QNetworkReply *reply=manager->get(request);
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        QTimer::singleShot(5000,&loop,&QEventLoop::quit);
        loop.exec();
        if(reply->error() != QNetworkReply::NoError) QMessageBox::critical(this,"错误","无法获取默认用户名。错误"+reply->errorString());
        else {ui->lineEdit_github_pat->setText(reply->readAll());ui->lineEdit_github->setText("nnpyro1");}
    }
    return true;
}
