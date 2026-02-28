#include "dialog_schedule.h"
#include "ui_dialog_schedule.h"

Dialog_schedule::Dialog_schedule(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_schedule)
{
    ui->setupUi(this);
    
    connect(ui->radioButton_type_time,&QRadioButton::clicked,this,&Dialog_schedule::on_type_change);
    connect(ui->radioButton_type_datetime,&QRadioButton::clicked,this,&Dialog_schedule::on_type_change);
    connect(ui->radioButton_type_after,&QRadioButton::clicked,this,&Dialog_schedule::on_type_change);
    connect(this,&Dialog_schedule::accepted,this,&Dialog_schedule::on_save);
}

Dialog_schedule::~Dialog_schedule()
{
    delete ui;
}


void Dialog_schedule::on_type_change(){
    if(ui->radioButton_type_time->isChecked()){
        ui->widget_time->setEnabled(true);
        ui->widget_datetime->setEnabled(false);
        ui->widget_after->setEnabled(false);
    }
    if(ui->radioButton_type_datetime->isChecked()){
        ui->widget_time->setEnabled(false);
        ui->widget_datetime->setEnabled(true);
        ui->widget_after->setEnabled(false);
    }
    if(ui->radioButton_type_after->isChecked()){
        ui->widget_time->setEnabled(false);
        ui->widget_datetime->setEnabled(false);
        ui->widget_after->setEnabled(true);
    }
}


void Dialog_schedule::on_save(){
    Schedule *schedule = new Schedule;
    if(ui->radioButton_type_time->isChecked()){
        int rpt = 0;
        if(ui->checkBox_repeat_1->isChecked()){
            rpt |= Schedule::rp_Monday;
        }
        if(ui->checkBox_repeat_2->isChecked()){
            rpt |= Schedule::rp_Tuesday;
        }
        if(ui->checkBox_repeat_3->isChecked()){
            rpt |= Schedule::rp_Wednesday;
        }
        if(ui->checkBox_repeat_4->isChecked()){
            rpt |= Schedule::rp_Thursday;
        }
        if(ui->checkBox_repeat_5->isChecked()){
            rpt |= Schedule::rp_Friday;
        }
        if(ui->checkBox_repeat_6->isChecked()){
            rpt |= Schedule::rp_Saturday;
        }
        if(ui->checkBox_repeat_7->isChecked()){
            rpt |= Schedule::rp_Sunday;
        }
        schedule->set(ui->timeEdit->time(),(Schedule::Repeat)rpt);
        emit saved(schedule);
        close();
    }
    if(ui->radioButton_type_datetime->isChecked()){
        schedule->set(ui->dateTimeEdit->dateTime());
        emit saved(schedule);
        close();
    }
    if(ui->radioButton_type_after->isChecked()){
        schedule->set(ui->doubleSpinBox_after->value()*60);
        emit saved(schedule);
        close();
    }
}
