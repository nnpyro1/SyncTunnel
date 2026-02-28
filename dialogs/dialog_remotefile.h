#ifndef DIALOG_REMOTEFILE_H
#define DIALOG_REMOTEFILE_H

#include <QDialog>
#include <QSet>

namespace Ui {
class Dialog_remoteFile;
}

class Dialog_remoteFile : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_remoteFile(QWidget *parent = nullptr);
    ~Dialog_remoteFile();
    
signals:
//    void send(QByteArray data,bool e=1,int d=-1);
    void request_fileList(QString path);
    void ok(QString file);
    
public:
    void setFileFolder(QString dir,QSet<QPair<bool,QString>> files);//bool代表是否是文件
    
private slots:
    void on_doubleClick();
    
private:
    Ui::Dialog_remoteFile *ui;
    QString currentDir;
};

#endif // DIALOG_REMOTEFILE_H
