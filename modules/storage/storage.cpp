#include "storage.h"
#include <QEventLoop>
#include <QTimer>
#include <QUrlQuery>
#include <../../libary/Qt-AES/qaesencryption.h>
#include <QJsonObject>
#include <QJsonDocument>


Storage::Storage(){
    
}

Storage::~Storage(){
    
}


/*bool Storage::upload(QByteArray file){
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://file.io"));
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType,this);
    
    //构造请求体
    QHttpPart p1;
    p1.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant(QString("form-data; name=\"file\"; filename=\"file\"")));
    p1.setBody(file);
    multiPart->append(p1);
    request.setRawHeader("User-Agent", "SyncTunnel-File Hang-up Service");
    
    //发起请求
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);
    
    //等待回复
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    QTimer timer;
    connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    timer.start(5000);
    loop.exec();
    
    //处理请求和错误
    QString result;
    if(reply->error() != QNetworkReply::NoError){
        qCritical()<<QString("Storage::upload(QByteArray) NetworkError. Details:%1").arg(reply->errorString());
        result="<NETWORK ERROR>return value:"+reply->readAll();
        qDebug()<<"|__var:result:"<<result;
        reply->deleteLater();
        return  "";
    }
    result = reply->readAll();
    reply->deleteLater();
    return result;
}*/

/*bool Storage::upload(const QByteArray &file){
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    //上传文件
    QNetworkRequest request(QUrl("https://www.file.io"));
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType,this);
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader,QString("form-data; name=\"file\""));
    part.setBody(file);
    multiPart->append(part);
    QNetworkReply *reply = manager->post(request,multiPart);
    
    //等待上传完成
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    
    //解析
    if(reply->error() != QNetworkReply::NoError){
        qCritical()<<"Storage::upload Error:Cannot upload file.Details:"<<reply->errorString();
        return false;
    }
    
    QByteArray response=reply->readAll();
    qDebug()<<"Storage::upload Debug var:response="<<response;
}*/

/*bool Storage::upload(const QByteArray &file){
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    //加密文件
    QAESEncryption encryption(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray encoded = encryption.encode(file,QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256),QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5));
    
    //上传文件
    QNetworkRequest request(QUrl("https://textdb.online/update"));
    QUrlQuery query;
    query.addQueryItem("key","nnpyro-SyncTunnel-FileHangup-userdata_"+username);
    query.addQueryItem("value","");
    
}*/

bool Storage::upload(const QByteArray &file){    
    //文件分片
    const int SPC = 1024 */* 1024 **/ 32;
//    const int SPC = 64;
    QList<QByteArray> list;
    for(int startpos=0;startpos<file.size();){
        list.append(file.mid(startpos,qMin(SPC,file.size()-startpos)));
        startpos += SPC;
    }
    
    //创建对象
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkRequest request;
    
    //构造请求
    request.setRawHeader("Authorization","Basic "+(QString("%1:%2").arg(username,pwd).toUtf8().toBase64()));
    request.setHeader(QNetworkRequest::UserAgentHeader,"NNPYRO SyncTunnel FileHangUp Service");
    QJsonObject json;
    json["message"]="这个文件是SyncTunnel的文件挂起服务所需的文件，请不要修改或删除";
    
    
    
    //设置统计文件
    request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/info.txt").arg(username)));
    json["content"]=QString(QString::number(list.length()).toUtf8().toBase64());
    QNetworkReply *reply = manager->put(request,QJsonDocument(json).toJson());
    
    //等待响应
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    
    if(1)qDebug()<<"Storage::upload Debug var:reply="<<reply->readAll();
    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Storage::upload Error:"<<reply->errorString();
        manager->deleteLater();
        return false;
    }
    reply->deleteLater();
    
    
    
    int count=0;
    foreach(auto i , list){
        request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/%2.nprivate").arg(username).arg(count)));
        json["content"]=QString((i).toBase64());
        QNetworkReply *reply = manager->put(request,QJsonDocument(json).toJson());
        
        //等待响应
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        
        if(1)qDebug()<<"Storage::upload Debug var:reply="<<reply->readAll();
        if(reply->error() != QNetworkReply::NoError){
            qDebug()<<"Storage::upload Error:"<<reply->errorString();
            manager->deleteLater();
            return false;
        }
        count++;
        reply->deleteLater();
    }
    
    return true;
}


bool Storage::remove(){
    bool flag = true;
    //获取文件个数
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkRequest request;
    request.setRawHeader("Authorization","Basic "+(QString("%1:%2").arg(username,pwd).toUtf8().toBase64()));
    request.setHeader(QNetworkRequest::UserAgentHeader,"NNPYRO SyncTunnel FileHangUp Service");
    QJsonObject json;
    json["message"] = "SyncTunnel应用程序自动删除";
    request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/info.txt").arg(username)));
    QNetworkReply *reply = manager->get(request);
    //等待响应
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //解析
    int file_total = QString(QByteArray::fromBase64(QJsonDocument::fromJson(reply->readAll()).object()["content"].toString().toUtf8())).toInt();
    
    //获取文件sha&删除文件
    for(int i=0;i<file_total;i++){
        request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/%2.nprivate").arg(username).arg(i)));
        QNetworkReply *reply = manager->get(request);
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<"Storage::remove error:"<<reply->errorString();
            flag=false;
        }
        //解析文件sha
        QString sha = QString(QJsonDocument::fromJson(QByteArray(reply->readAll())).object()["sha"].toString());
        if(sha.isEmpty()){
            qDebug()<<"Storage::remove:empty sha";
            flag=false;
        }
        //删除文件
        json["sha"]=sha;
        reply->deleteLater();
        reply = manager->sendCustomRequest(request,"DELETE",QJsonDocument(json).toJson());
        QEventLoop loop2;
        connect(reply,&QNetworkReply::finished,&loop2,&QEventLoop::quit);
        loop2.exec();
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<"Storage::remove error:"<<reply->errorString();
            flag=false; 
        }
        reply->deleteLater();
    }
    
    
    //删除info.txt
    request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/info.txt").arg(username)));
    reply = manager->get(request);
    QEventLoop loop2;
    connect(reply,&QNetworkReply::finished,&loop2,&QEventLoop::quit);
    loop2.exec();
    if(reply->error()!=QNetworkReply::NoError){
        qDebug()<<"Storage::remove error:"<<reply->errorString();
        flag=false;
    }
    //解析文件sha
    QString sha = QString(QJsonDocument::fromJson(QByteArray(reply->readAll())).object()["sha"].toString());
    if(sha.isEmpty()){
        qDebug()<<"Storage::remove:empty sha";
        flag=false;
    }
    //删除文件
    json["sha"]=sha;
    reply->deleteLater();
    reply = manager->sendCustomRequest(request,"DELETE",QJsonDocument(json).toJson());
    QEventLoop loop3;
    connect(reply,&QNetworkReply::finished,&loop3,&QEventLoop::quit);
    loop3.exec();
    if(reply->error()!=QNetworkReply::NoError){
        qDebug()<<"Storage::remove error:"<<reply->errorString();
        flag=false; 
    }
    reply->deleteLater();
    
    return flag;
}


QByteArray Storage::get(){
    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkRequest request;
    
    //获取info.txt
    //构造请求
    request.setRawHeader("Authorization","Basic "+(QString("%1:%2").arg(username,pwd).toUtf8().toBase64()));   
    request.setHeader(QNetworkRequest::UserAgentHeader,"NNPYRO SyncTunnel FileHangUp Service");
    request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/info.txt").arg(username)));
    QNetworkReply *reply = manager->get(request);
    
    //处理响应
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    
    //分别请求每个文件
    QByteArray return_value;
    int total = QString(QByteArray::fromBase64(QJsonDocument::fromJson(reply->readAll()).object()["content"].toString().toUtf8())).toInt();
    for(int i=0;i<total;i++){
        request.setUrl(QUrl(QString("https://api.github.com/repos/%1/synctunnel-filehangup/contents/%2.nprivate").arg(username).arg(i)));
        QNetworkReply *reply = manager->get(request);
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<"Storage::get error:"<<reply->errorString();
        }
        //解析文件
        QByteArray response = reply->readAll();
        qDebug()<<"Storage::get var:response="<<response;
        QByteArray content = QByteArray::fromBase64(QJsonDocument::fromJson(QByteArray(response)).object()["content"].toString().toUtf8());
        if(content.isEmpty()){
            qDebug()<<"Storage::get:empty content";
        }
        return_value.append(content);
        reply->deleteLater();
    }
    
    manager->deleteLater();
    reply->deleteLater();
    return return_value;
}