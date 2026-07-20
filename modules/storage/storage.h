#ifndef STORAGE_H
#define STORAGE_H

#include "general.h"
#include "qnetworkaccessmanager.h"
#include <QObject>

class Storage : public QObject
{
    Q_OBJECT
public:
    enum ProcessingState{
        Idle,
        //上传文件
        Init,
    };
    Q_ENUM(ProcessingState)
    
    explicit Storage(QObject *parent = nullptr);
    virtual ~Storage();
    
    void setPassport(QString username,QString pwd);
    
    Result upload(QByteArray data);
    QByteArray download();
    
signals:
    void progressUpdated(Storage::ProcessingState state,double progress);
    
protected:
    QByteArray encode(QByteArray data);
    QByteArray decode(QByteArray data);
    
    Result uploadSmallFile(QByteArray data,QUrl uploadUrl,QString r2Key); 
    Result uploadLargeFile(QByteArray data,QString r2Key,QString uploadId,quint32 partSize,quint32 totalParts,QList<QUrl> initalUrls,QString ownerToken);
    
    Result uploadToCf(QByteArray data,QUrl uploadUrl,QByteArray *outputETag = nullptr);
    
protected:
    Q_SIGNAL void stepProgressUpdated(double progress);
    
protected:
    QNetworkAccessManager *manager;
    QString username,pwd;
    QString userPassport;
    
    const QString URL_KV = "https://mantledb.sh/v2";
};

#endif // STORAGE_H
