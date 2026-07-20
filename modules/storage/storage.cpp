#include "storage.h"
#include "qeventloop.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qnetworkreply.h"
#include "utils.h"

Storage::Storage(QObject *parent)
    : QObject{parent}
    ,manager(new QNetworkAccessManager(this))
{
    
}


Storage::~Storage(){
    manager->deleteLater();
}


void Storage::setPassport(QString username, QString pwd){
    this->username=username;
    this->pwd=pwd;
    userPassport=QCryptographicHash::hash((username+"@"+pwd).toUtf8(),QCryptographicHash::Sha256).toHex();
}


Result Storage::upload(QByteArray data){
    QObject memoryManager;
    //发送init请求
    QNetworkRequest req;
    req.setUrl(QUrl("https://storage.to/api/upload/init"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    req.setRawHeader("X-Visitor-Token","synctunnel_visitor_token-"+userPassport.toUtf8());
    QJsonObject body;
    body.insert("filename",userPassport+"-encrypted.bin");
    body.insert("content_type","application/octet-stream");
    body.insert("size",data.size());
    QNetworkReply *reply = manager->post(req, QJsonDocument(body).toJson());
    reply->setParent(&memoryManager);
    emit progressUpdated(Init,0.01);
    //等待直到请求完成
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //错误处理
    if(reply->error()!=QNetworkReply::NoError){
        return Result("Init\n"+reply->errorString());
    }
    auto response = QJsonDocument::fromJson(reply->readAll()).object();
    if(response["success"] != true){
        return Result("Init\n"+reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()+"\n"+response["error"].toString());
    }
    //解析请求
    if(response["type"]=="single"){//单分片上传
        return uploadSmallFile(data,QUrl(response["upload_url"].toString()),response["r2_key"].toString());
    }
    if(response["type"]=="multipart"){//单分片上传
        auto iurl = response["inital_urls"].toObject();
        QList<QUrl> initalUrlList;
        initalUrlList.resize(iurl.size());
        for(auto key:iurl.keys()){
            initalUrlList[key.toInt()-1]=iurl[key].toString();
        }
        return uploadLargeFile(data,response["r2_key"].toString(),response["upload_id"].toString(),response["part_size"].toInt(),response["total_parts"].toInt(),initalUrlList,response["owner_token"].toString());
    }
    else{
        return Result("Init\nUnknown "+response["type"].toString());
    }
}


QByteArray Storage::download(){
    
}


QByteArray Storage::encode(QByteArray data){
    return Utils::encode(data,pwd);
}


QByteArray Storage::decode(QByteArray data){
    return Utils::decode(data,pwd);
}


Result Storage::uploadSmallFile(QByteArray data,QUrl uploadUrl,QString r2Key){
    
}


Result Storage::uploadLargeFile(QByteArray data,QString r2Key,QString uploadId,quint32 partSize,quint32 totalParts,QList<QUrl> initalUrls,QString ownerToken){
    
}


Result Storage::uploadToCf(QByteArray data, QUrl uploadUrl, QByteArray *outputETag){
    QObject memoryManager;
    QNetworkRequest req(uploadUrl);
    req.setHeader(req.ContentTypeHeader,"application/octet-stream");
    auto reply = manager->put(req,data);
    reply->setParent(&memoryManager);
    //等待结果
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    connect(reply,&QNetworkReply::uploadProgress,&memoryManager,[this](qint64 d,qint64 t){emit stepProgressUpdated(d*1./t);});
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        return Result("uploadToCf\n"+reply->errorString());
    }
    //数据处理
    //###
}
