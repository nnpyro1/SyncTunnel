#ifndef WIZARD_STARTUP_H
#define WIZARD_STARTUP_H

#include <QWizard>
#include <QString>

namespace Ui {
class Wizard_startup;
}

class Wizard_startup : public QWizard
{
    Q_OBJECT
    
public:
    explicit Wizard_startup(QWidget *parent = nullptr);
    ~Wizard_startup();
    
signals:
    void settingsSaved(
            QString user_name,
            QString pwd,
            QString github_username,
            QString pat
            );
protected:
    bool validateCurrentPage() override;
private:
    Ui::Wizard_startup *ui;
};

#endif // WIZARD_STARTUP_H
