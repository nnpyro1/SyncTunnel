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

void Dialog_selectSyncDst::setup(Devices _devices)
{
    this->devices = _devices;
    //添加设备
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableWidget->setColumnWidth(0,30);
    int index = 0;
    foreach(auto i,devices){
        devid_t devId = qToBigEndian(getIdByDevice(i));
        QCheckBox *checkBox = new QCheckBox;
        checkBoxList.append(checkBox);
        checkBox->setCheckState(Qt::Checked);
        ui->tableWidget->insertRow(index);
        ui->tableWidget->setCellWidget(index,0,checkBox);
        ui->tableWidget->setItem(index,1,new QTableWidgetItem(getStringByDeviceId(devId)));
        ui->tableWidget->setItem(index,2,new QTableWidgetItem(QString("%1").arg(i.operator QString const())));
        index++;
    }
    
    //设置
    connect(this,&Dialog_selectSyncDst::accepted,this,[this]{
        QSet<devid_t> result;
//        int index=0;
//        foreach(auto i , checkBoxList){
//            if(i->checkState() == Qt::Checked)
//                result.append(this->devices[index]);
//                index++;}
        for(int i=0;i<ui->tableWidget->rowCount();i++){
            if(checkBoxList[i]->isChecked()){
                devid_t devid = qFromBigEndian(*(devid_t*)(QByteArray::fromBase64(ui->tableWidget->item(i,1)->text().toUtf8()).constData()));
                result.insert(devid);
            }
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
    // int index=0;
    // foreach(auto i , checkBoxList){
    //     if(devices[index].flag != Communication::DFHNDevice)i->setCheckState(Qt::Checked);
    //     else i->setCheckState(Qt::Unchecked);
    //     index++;
    // }
    on_pushButton_selectAll_clicked();
}

void Dialog_selectSyncDst::on_finished(){
    
}

