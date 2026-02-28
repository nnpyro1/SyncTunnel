#include "dialog_remotefile.h"
#include "ui_dialog_remotefile.h"
#include <QFileIconProvider>
#include <QDir>

Dialog_remoteFile::Dialog_remoteFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_remoteFile)
{
    ui->setupUi(this);
    
    connect(ui->listWidget,&QListWidget::doubleClicked,this,&Dialog_remoteFile::on_doubleClick);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[this]{
        if(ui->listWidget->currentRow()==-1) return;
        if(1) {
            emit ok(QDir(currentDir).filePath(ui->listWidget->currentItem()->text()));
        }
        close();
    });
}

Dialog_remoteFile::~Dialog_remoteFile()
{
    delete ui;
}


void Dialog_remoteFile::setFileFolder(QString dir, QSet<QPair<bool, QString>> files)
{
    ui->label_status->setText("就绪");
    currentDir=dir;
    ui->listWidget->clear();
    ui->label_currentDir->setText(dir);
    QSet<QPair<bool, QString>>::const_iterator it = files.begin();
    ui->listWidget->addItem("..");
    for(;it!=files.end();it++){
        if(it->first){//是文件
            QListWidgetItem *item = new QListWidgetItem;
            item->setIcon(QFileIconProvider().icon(QFileIconProvider::File));
            item->setText(it->second);
            item->setData(Qt::UserRole,"file");
            ui->listWidget->addItem(item);
        }
        else{
            QListWidgetItem *item = new QListWidgetItem;
            item->setIcon(QFileIconProvider().icon(QFileIconProvider::Folder));
            item->setText(it->second);
            item->setData(Qt::UserRole,"folder");
            ui->listWidget->addItem(item);
        }
    }
}


void Dialog_remoteFile::on_doubleClick()
{
    int row = ui->listWidget->currentRow();
    if(row == -1){
        return;
    }
    QListWidgetItem *item = ui->listWidget->item(row);
    
    if(item->data(Qt::UserRole).toString() == "folder"|item->text()==".."){//目录
        ui->label_status->setText("正在请求文件数据...");
        emit request_fileList(QDir(currentDir).filePath(item->text()));
    }
    else{
        ui->label_status->setText("请点击YES来确定复制该文件。稍后您可以从该设备请求文件以获取该文件");
    }
//    ui->listWidget->clear();
    
}

