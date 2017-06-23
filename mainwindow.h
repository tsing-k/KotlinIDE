#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QTabWidget;
class QTreeWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotNewFile(const QString &str = QString());   //新建文件

private:
    Ui::MainWindow *ui;

    QTreeWidget     *mProjectTreeWidget;    //工程文件列表
    QTabWidget      *mCodeTabWidget;        //代码区
};

#endif // MAINWINDOW_H
