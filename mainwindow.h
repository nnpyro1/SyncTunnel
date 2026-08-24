#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <viewmodel.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ViewModel *vm, QWidget *parent = nullptr, std::function<void(QString)> msgLogger=nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    ViewModel *vm;
};
#endif // MAINWINDOW_H
