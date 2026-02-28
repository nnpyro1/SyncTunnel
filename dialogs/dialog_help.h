#ifndef DIALOG_HELP_H
#define DIALOG_HELP_H

#include <QDialog>

namespace Ui {
class Dialog_help;
}

class Dialog_help : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_help(QWidget *parent = nullptr);
    ~Dialog_help();
    
private:
    Ui::Dialog_help *ui;
};

#endif // DIALOG_HELP_H
