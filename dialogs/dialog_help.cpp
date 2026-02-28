#include "dialog_help.h"
#include "ui_dialog_help.h"

Dialog_help::Dialog_help(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_help)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
}

Dialog_help::~Dialog_help()
{
    delete ui;
}
