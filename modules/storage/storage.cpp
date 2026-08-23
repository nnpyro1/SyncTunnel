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
    ninfo<<"Start uploading";
    QObject memoryManager;
    //发送init请求
    ninfo<<"Request:init";
    QNetworkRequest req;
    req.setUrl(QUrl("https://storage.to/api/upload/init"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    req.setRawHeader("X-Visitor-Token",userPassport.toUtf8());
    QJsonObject body;
    body.insert("filename",userPassport+".bin");
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
        ncritical<<"Error:"<<reply->errorString();
        return Result("Init\n"+reply->errorString());
    }
    auto response = QJsonDocument::fromJson(reply->readAll()).object();
    if(response["success"] != true){
        ncritical<<"Error:"<<response;
        return Result("Init\n"+reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()+"\n"+response["error"].toString());
    }
    //解析请求
    if(response["type"]=="single"){//单分片上传
        return uploadSmallFile(data,QUrl(response["upload_url"].toString()),response["r2_key"].toString());
    }
    if(response["type"]=="multipart"){//单分片上传
        auto iurl = response["initial_urls"].toObject();
        QList<QUrl> initialUrlList;
        initialUrlList.resize(iurl.size());
        for(auto key:iurl.keys()){
            initialUrlList[key.toInt()-1]=iurl[key].toString();
        }
        return uploadLargeFile(data,response["r2_key"].toString(),response["upload_id"].toString(),response["part_size"].toInt(),response["total_parts"].toInt(),initialUrlList,response["owner_token"].toString());
    }
    else{
        return Result("Init\nUnknown "+response["type"].toString());
    }
}


QByteArray Storage::download(){
    return "";
}


QByteArray Storage::encode(QByteArray data){
    return Utils::encode(data,username,pwd);
}


QByteArray Storage::decode(QByteArray data){
    return Utils::decode(data,username,pwd);
}


Result Storage::uploadSmallFile(QByteArray data,QUrl uploadUrl,QString r2Key){
    ninfo<<"uploadSmallFile";
    // connect(this,&Storage::stepProgressUpdated,this,[this](double d){
    //     emit progressUpdated(Upload,d*0.98);
    // });
    // Result r1=uploadToCf(data,uploadUrl,nullptr);
    // if(r1.is_succeeded){
    //     emit progressUpdated(Confirm,0.99);
    //     return confirm(userPassport+".bin",data.size(),r2Key);
    // }
    // else return r1;
    
    //1 上传CF
    auto c1=connect(this,&Storage::stepProgressUpdated,this,[this](double d){
        emit progressUpdated(Upload,d*0.97);
    });
    Result r1 = uploadToCf(data,uploadUrl);
    disconnect(c1);
    if(!r1.is_succeeded){
        return r1;
    }
    //2 confirm
    emit progressUpdated(Confirm,0.98);
    QString id;
    Result r2 = confirm(userPassport+".bin",data.size(),r2Key,&id);
    if(!r2.is_succeeded){
        return r2;
    }
    //3 setDownloadUrl
    emit progressUpdated(SetDownloadUrl,0.99);
    Result r3 = setDownloadUrl("https://storage.to/"+id);
    return r3;
}


Result Storage::uploadLargeFile(QByteArray data,QString r2Key,QString uploadId,quint32 partSize,quint32 totalParts,QList<QUrl> initalUrls,QString ownerToken){
    return Result("uploadLargeFile\nError\n开发者偷了点懒，目前这个功能还没有编完~\n解决方案1:上传小于50MB的文件\n解决方案2::可以到Github上给本项目贡献一个PR来完善这个功能");
}


Result Storage::uploadToCf(QByteArray data, QUrl uploadUrl, QString *outputETag){
    ninfo<<"Uploading to CloudFlare R2";
    QObject memoryManager;
    QNetworkRequest req(uploadUrl);
    req.setHeader(req.ContentTypeHeader,"application/octet-stream");
    req.setRawHeader("Host", uploadUrl.host().toUtf8());
    auto reply = manager->put(req,data);
    // reply->setParent(&memoryManager);
    //等待结果
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    connect(reply,&QNetworkReply::uploadProgress,&memoryManager,[this, &data](qint64 d,qint64 t){emit stepProgressUpdated(d*1./data.size());});
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        ncritical<<"Error:"<<reply->errorString();
        reply->deleteLater();
        return Result("uploadToCf\n"+reply->errorString());
    }
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300) {
        ncritical<<QString::number(statusCode);
        reply->deleteLater();
        return Result("uploadToCf\nHTTP error\n" + QString::number(statusCode));
    }
    
    //数据处理
    QString etag = reply->header(QNetworkRequest::ETagHeader).toString();
    if(outputETag){
        *outputETag = etag;
    }
    ninfo<<"Succeded.ETag="<<etag;
    reply->deleteLater();
    return Result();
}


Result Storage::confirm(QString filename, qsizetype size, QString r2Key,QString *outputFileId){
    ninfo<<"Request:confirm";
    QObject memoryManager;
    QNetworkRequest req(QUrl("https://storage.to/api/upload/confirm"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    req.setRawHeader("X-Visitor-Token",userPassport.toUtf8());
    QJsonObject reqJson;
    reqJson.insert("filename",filename);
    reqJson.insert("size",size);
    reqJson.insert("content_type","application/octet-stream");
    reqJson.insert("r2_key",r2Key);
    auto reply = manager->post(req,QJsonDocument(reqJson).toJson());
    reply->setParent(&memoryManager);
    //等待结果
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        ncritical<<"Error:"<<reply->errorString();
        return Result("confirm\n"+reply->errorString());
    }
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300) {
        ncritical<<QString::number(statusCode);
        return Result("confirm\nHTTP error\n" + QString::number(statusCode));
    }
    QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    if(json["success"]!=true){
        ncritical<<"Error:"<<json;
        return Result("confirm\n"+json["error"].toString());
    }
    ninfo<<"Succeeded. Response = "<<json;
    if(outputFileId){
        *outputFileId=json["id"].toString();
    }
    return Result();
}


Result Storage::setDownloadUrl(QUrl downloadUrl){
    QObject memoryManager;
    QNetworkRequest req(QUrl("https://synctunnel-interface.nnpyro.cc.cd/api/storage/set"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");    
    QJsonObject reqJson;
    reqJson["passport"]=userPassport;
    reqJson["download_url"]=downloadUrl.toString();
    QNetworkReply *reply = manager->post(req,QJsonDocument(reqJson).toJson());
    reply->setParent(&memoryManager);
    ninfo<<"Request:setDownloadUrl";
    //阻塞等待
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        ncritical<<"Error:"<<reply->errorString();
        return Result("setDownloadUrl\n"+reply->errorString());
    }
    QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    if(json["success"].toBool() != true){
        ncritical<<"Error:"<<json;
        return Result("setDownloadUrl\n"+json["error"].toString());
    }
    return Result();
}


Result Storage::getDownloadUrl(QUrl &output){
    QObject memoryManager;
    QNetworkRequest req(QUrl("https://synctunnel-interface.nnpyro.cc.cd/api/storage/get"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QJsonObject reqJson;
    reqJson["passport"]=userPassport;
    QNetworkReply *reply = manager->post(req,QJsonDocument(reqJson).toJson());
    reply->setParent(&memoryManager);
    ninfo<<"Request:getDownloadUrl";
    //阻塞等待
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        ncritical<<"Error:"<<reply->errorString();
        return Result("getDownloadUrl\n"+reply->errorString());
    }
    QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    if(json["success"].toBool() != true){
        ncritical<<"Error:"<<json;
        return Result("getDownloadUrl\n"+json["error"].toString());
    }
    output=json["download_url"].toString();
    return Result();
}


Result Storage::setExpiry(int days, QString fileId){
    QObject memoryManager;
    QNetworkRequest req(QUrl(QString("https://storage.to/api/file/%1/expiry").arg(fileId)));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QJsonObject reqJson;
    reqJson["days"]=days;
    QNetworkReply *reply = manager->post(req,QJsonDocument(reqJson).toJson());
    reply->setParent(&memoryManager);
    ninfo<<"Request:setExpiry";
    //阻塞等待
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    //错误处理
    if(reply->error()!=reply->NoError){
        ncritical<<"Error:"<<reply->errorString();
        return Result("setExpiry\n"+reply->errorString());
    }
    QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    // if(json["success"].toBool() != true){
    //     ncritical<<"Error:"<<json;
    //     return Result("setExpiry\n"+json["error"].toString());
    // }
    // output=json["download_url"].toString();
    return Result();
}
