#ifndef DIALOG_SCHEDULE_H
#define DIALOG_SCHEDULE_H

#include <QDialog>
#include <schedule.h>

namespace Ui {
class Dialog_schedule;
}

class Dialog_schedule : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_schedule(QWidget *parent = nullptr);
    ~Dialog_schedule();
    
private slots:
    void on_type_change();
    void on_save();
    
signals:
    void saved(Schedule *schedule);
    
private:
    Ui::Dialog_schedule *ui;
};

#endif // DIALOG_SCHEDULE_H
