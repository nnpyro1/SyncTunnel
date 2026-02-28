#include "dialog_selectsyncdst.h"
#include "ui_dialog_selectsyncdst.h"


Dialog_selectSyncDst::Dialog_selectSyncDst(QWidget *parent)
    :QDialog(parent),ui(new Ui::Dialog_selectSyncDst)
{
    ui->setupUi(this);
    
}

Dialog_selectSyncDst::~Dialog_selectSyncDst()
{
    delete ui;
}

void Dialog_selectSyncDst::setup(QList<Communication::device> devices)
{
    this->devices = devices;
    //添加设备
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableWidget->setColumnWidth(0,30);
    int index = 0;
    foreach(auto i,devices){
        QCheckBox *checkBox = new QCheckBox;
        checkBoxList.append(checkBox);
        checkBox->setCheckState(Qt::Checked);
        ui->tableWidget->insertRow(index);
        ui->tableWidget->setCellWidget(index,0,checkBox);
        ui->tableWidget->setItem(index,1,new QTableWidgetItem(QString("%1").arg(devices[index].operator QString const())));
        index++;
    }
    
    //设置
    connect(this,&Dialog_selectSyncDst::accepted,this,[devices,this]{
        QList<Communication::device> result;
//        int index=0;
//        foreach(auto i , checkBoxList){
//            if(i->checkState() == Qt::Checked)
//                result.append(this->devices[index]);
//                index++;}
        for(int i=0;i<checkBoxList.size();i++){
            if(checkBoxList[i]->checkState()==Qt::Checked)
                result.append(devices[i]);
        }
        emit syncdstDecided(result);close();
    });
}


void Dialog_selectSyncDst::on_pushButton_unselectAll_clicked(){
    foreach(auto i , checkBoxList){
        i->setCheckState(Qt::Unchecked);
    }
}

void Dialog_selectSyncDst::on_pushButton_selectAll_clicked(){
    foreach(auto i , checkBoxList){
        i->setCheckState(Qt::Checked);
    }
}

void Dialog_selectSyncDst::on_pushButton_selectWithoutDFHN_clicked(){
    int index=0;
    foreach(auto i , checkBoxList){
        if(devices[index].flag != Communication::DFHNDevice)i->setCheckState(Qt::Checked);
        else i->setCheckState(Qt::Unchecked);
        index++;
    }
}

void Dialog_selectSyncDst::on_finished(){
    
}

