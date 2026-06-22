#ifndef DIALOG_SELECTSYNCDST_H
#define DIALOG_SELECTSYNCDST_H

#include <QDialog>
#include <modules/communication/communication.h>
#include <QList>
#include <QCheckBox>

namespace Ui {
class Dialog_selectSyncDst;
}

class Dialog_selectSyncDst : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_selectSyncDst(QWidget *parent);
    ~Dialog_selectSyncDst();
    void setup(Devices devices);
    
private slots:
    void on_pushButton_unselectAll_clicked();
    
    void on_pushButton_selectAll_clicked();
    
    void on_pushButton_selectWithoutDFHN_clicked();
    
    void on_finished();
    
signals:
    void syncdstDecided(Devices result);
    
private:
    Ui::Dialog_selectSyncDst *ui;
    QList<QCheckBox *> checkBoxList;
    Devices devices;
};

#endif // DIALOG_SELECTSYNCDST_H
