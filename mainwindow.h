#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class QTabWidget;
class QTreeWidget;
class CodeEditor;
class QTextEdit;
class QTreeWidgetItem;
class QProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void closeFile(int index);

    void textModified(bool changed);

    void textSaved();

    void projectTreeContextMenu(const QPoint &pos);

    void on_actionNew_File_triggered();

    void on_actionOpen_File_triggered();

    void projectFileDoubleClicked(QTreeWidgetItem *item, int column);

    void readOutput();

    void buildError(QProcess::ProcessError error);

private slots:
    void slotOpenFile(bool);

    void slotAddFileToProject(bool);    //添加已有文件

    void slotCreateNewFileOfProject(bool);      //添加新文件

    void on_actionNew_Project_triggered();

    void on_actionOpen_Project_triggered();

    void on_actionBuild_triggered();

    void on_actionRun_triggered();

private:
    void setupUI();

    void initConnect();

    void newFile(const QString &str = QString());   //新建文件

    void openFile(const QString &file);

    int findEditorIndex(const CodeEditor *ptr);

    int findEditorIndex(const QString &tabText);

    void newProject(const QString &prjPath);

    void openProject(const QString &prjFile);

    void addFileToProject(const QString &file);

private:
    Ui::MainWindow *ui;

    QTreeWidget     *mProjectTreeWidget;    //工程文件列表
    QTabWidget      *mCodeTabWidget;        //代码区
    QTextEdit       *mOutputWidget;         //输出

    QMap<QString, QString>    mOpenFiles;     //保存打开的文件路径 <basename, 文件绝对路径>
    QString         mCurProjectPath;        //当前项目的路径

    QProcess        *mKotlinProcess;
};

#endif // MAINWINDOW_H
