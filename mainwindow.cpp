#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QTextEdit>
#include <QMenu>
#include <QInputDialog>
#include <QProcess>
#include <QRegExp>
#include <QTextCodec>

#define ModifiedMark    QString(" *")    //文件修改的标记
#define LineSep         QString("\r\n")
#define ProjectSuffix   QString(".ktp")  //工程文件后缀

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
  , mProjectTreeWidget(0)
  , mCodeTabWidget(0)
  , mOutputWidget(0)
  , mKotlinProcess(0)
{
    ui->setupUi(this);

    setupUI();
    initConnect();

    //mKotlinProcess->start("cmd.exe", QStringList() << "/c C:\\kotlinc\\bin\\kotlinc C:\\Users\\qdu\\Desktop\\kotlin\\main.kt -include-runtime -d C:\\Users\\qdu\\Desktop\\kotlin\\main.jar");
    //mKotlinProcess->start("cmd.exe", QStringList() << "/c" << "C:\\kotlinc\\bin\\kotlinc" << "C:\\Users\\qdu\\Desktop\\kotlin\\main.kt" << "-include-runtime -d" << "C:\\Users\\qdu\\Desktop\\kotlin\\main.jar");
}

MainWindow::~MainWindow()
{
    if(mKotlinProcess)
    {
        mKotlinProcess->close();
        delete mKotlinProcess;
    }
    delete mProjectTreeWidget;
    delete mCodeTabWidget;
    delete mOutputWidget;
    delete ui;
}

void MainWindow::newFile(const QString &str)
{
    CodeEditor *editor = new CodeEditor;
    mCodeTabWidget->addTab(editor, str);
    mCodeTabWidget->setCurrentIndex(mCodeTabWidget->count() - 1);
    editor->setFocus();
    connect(editor, &CodeEditor::modificationChanged, this, &MainWindow::textModified);
    connect(editor, &CodeEditor::requestSave, this, &MainWindow::textSaved);
}

void MainWindow::openFile(const QString &file)
{
    QFileInfo info(file);
    if(!info.exists())
    {
        QMessageBox::warning(this, tr("Open file"), tr("File doesn't exist!"));
        return;
    }
    QFile mFile(file);
    if(!mFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Open file"), tr("Open file \"%1\" failed!").arg(file));
        return;
    }
    QString strContent = mFile.readAll();
    mFile.close();

    CodeEditor *editor = new CodeEditor;
    editor->setPlainText(strContent);
    mCodeTabWidget->addTab(editor, info.fileName());
    mCodeTabWidget->setCurrentIndex(mCodeTabWidget->count() - 1);
    connect(editor, &CodeEditor::modificationChanged, this, &MainWindow::textModified);
    connect(editor, &CodeEditor::requestSave, this, &MainWindow::textSaved);

    mOpenFiles[info.fileName()] = file;
}

int MainWindow::findEditorIndex(const CodeEditor *ptr)
{
    int index = -1;
    for(int i = 0; i < mCodeTabWidget->count(); i++)
    {
        CodeEditor *editor = qobject_cast<CodeEditor *>(mCodeTabWidget->widget(i));
        if(editor == ptr)
        {
            index = i;
            break;
        }
    }

    return index;
}

int MainWindow::findEditorIndex(const QString &tabText)
{
    int index = -1;
    for(int i = 0; i < mCodeTabWidget->count(); i++)
    {
        if(tabText == mCodeTabWidget->tabText(i))
        {
            index = i;
            break;
        }
    }

    return index;
}

void MainWindow::newProject(const QString &prjPath)
{
    QFileInfo info(prjPath);
    mProjectTreeWidget->clear();
    QTreeWidgetItem *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, info.baseName());
    rootItem->setIcon(0, QIcon(":/image/folder.png"));

    QFile pFile(prjPath);
    if(pFile.open(QIODevice::WriteOnly))
    {
        pFile.close();
    }

    mProjectTreeWidget->addTopLevelItem(rootItem);
    mCurProjectPath = info.absolutePath();
}

void MainWindow::openProject(const QString &prjFile)
{
    QFileInfo info(prjFile);
    if(!info.exists())
    {
        QMessageBox::warning(this, tr("Open project"), tr("Project doesn't exist!"));
        return;
    }
    mProjectTreeWidget->clear();
    for(int i = mCodeTabWidget->count() - 1; i >= 0; i--)
    {
        delete mCodeTabWidget->widget(i);
        mCodeTabWidget->removeTab(i);
    }
    QTreeWidgetItem *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, info.baseName());
    rootItem->setIcon(0, QIcon(":/image/folder.png"));
    mProjectTreeWidget->addTopLevelItem(rootItem);

    QFile pFile(prjFile);
    if(pFile.open(QIODevice::ReadOnly))
    {
        QString prjInfo = pFile.readAll();
        pFile.close();
        QStringList fileList = prjInfo.split(LineSep, QString::SkipEmptyParts);
        foreach (QString file, fileList) {
            openFile(file);
            addFileToProject(file);
        }
    }
    mCurProjectPath = info.absolutePath();
}

void MainWindow::addFileToProject(const QString &file)
{
    QFileInfo info(file);
    QTreeWidgetItem *rootItem = mProjectTreeWidget->topLevelItem(0);
    if(rootItem)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, info.fileName());
        item->setIcon(0, QIcon(":/image/file.png"));
        rootItem->addChild(item);
    }
}

void MainWindow::closeFile(int index)
{
    CodeEditor *codeEditor = qobject_cast<CodeEditor *>(mCodeTabWidget->widget(index));
    if(codeEditor && codeEditor->document()->isModified())
    {
        if(QMessageBox::Ok == QMessageBox::warning(this, tr("Close"), tr("Save file before closing?"),
                                                   QMessageBox::Ok | QMessageBox::Cancel))
        {
            QString content = codeEditor->toPlainText();
            QString file = QFileDialog::getSaveFileName(this, tr("Save"));
            if(!file.isEmpty())
            {
                QFile mFile(file);
                if(mFile.open(QIODevice::WriteOnly))
                {
                    QByteArray datagram;
                    datagram.append(content);
                    mFile.write(datagram);
                }
            }
        }
    }

    mCodeTabWidget->removeTab(index);
}

void MainWindow::textModified(bool changed)
{
    if(!changed)
    {
        return;
    }

    CodeEditor *editor = qobject_cast<CodeEditor *>(sender());
    if(editor)
    {
        int idx = findEditorIndex(editor);
        if(idx >= 0)
        {
            QString tabText = mCodeTabWidget->tabText(idx);
            if(tabText.endsWith(ModifiedMark))
            {
                return;
            }
            tabText.append(ModifiedMark);
            mCodeTabWidget->setTabText(idx, tabText);
        }
    }
}

void MainWindow::textSaved()
{
    CodeEditor *editor = qobject_cast<CodeEditor *>(sender());
    if(editor)
    {
        QString content = editor->toPlainText();
        int idx = findEditorIndex(editor);
        if(idx >= 0)
        {
            QString tabText = mCodeTabWidget->tabText(idx);
            if(tabText.endsWith(ModifiedMark))
            {
                tabText.chop(ModifiedMark.length());
            }
            QString path = mOpenFiles.value(tabText, "NON");
            if("NON" == path)
            {
                QString file = QFileDialog::getSaveFileName(this, tr("Save"));
                if(!file.isEmpty())
                {
                    QFile mFile(file);
                    if(mFile.open(QIODevice::WriteOnly))
                    {
                        QByteArray datagram;
                        datagram.append(content);
                        mFile.write(datagram);
                    }
                    mFile.close();

                    QFileInfo info(file);
                    mOpenFiles[info.fileName()] = file;
                    mCodeTabWidget->setTabText(idx, info.fileName());
                }
            }
            else
            {
                QFile mFile(path);
                if(mFile.open(QIODevice::WriteOnly))
                {
                    QByteArray datagram;
                    datagram.append(content);
                    mFile.write(datagram);
                }
                mFile.close();
                mCodeTabWidget->setTabText(idx, tabText);
            }
        }
        editor->document()->setModified(false);
    }
}

void MainWindow::projectTreeContextMenu(const QPoint &pos)
{
    QMenu menu(mProjectTreeWidget);
    QTreeWidgetItem *curItem = mProjectTreeWidget->itemAt(pos);
    if(curItem)    //文件或文件夹
    {
        if(!curItem->parent())  //没有parent,则为root，即文件夹
        {
            QAction *actAddNewFile = new QAction(tr("Add new file..."));
            QAction *actAddExitingFile = new QAction(tr("Add exiting file..."));
            connect(actAddNewFile, &QAction::triggered, this, &MainWindow::slotCreateNewFileOfProject);
            connect(actAddExitingFile, &QAction::triggered, this, &MainWindow::slotAddFileToProject);
            menu.addAction(actAddNewFile);
            menu.addAction(actAddExitingFile);
        }
        else                    //有parent,则为文件
        {
            QAction *actOpenFile = new QAction("Open file");
            actOpenFile->setData(curItem->text(0));
            connect(actOpenFile, &QAction::triggered, this, &MainWindow::slotOpenFile);
        }
    }
    else
    {

    }
    menu.exec(QCursor::pos());
}

void MainWindow::on_actionNew_File_triggered()
{
    newFile("New File" + ModifiedMark);
}

void MainWindow::on_actionOpen_File_triggered()
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Browser"), QString());
    foreach (QString file, fileList) {
        openFile(file);
    }
}

void MainWindow::projectFileDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(item && item->parent())
    {
        int idx = findEditorIndex(item->text(column));
        if(idx >= 0)
        {
            mCodeTabWidget->setCurrentIndex(idx);
        }
        else
        {
            QString file = mCurProjectPath + "/" + item->text(column);
            openFile(file);
        }
    }
}

void MainWindow::readOutput()
{
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QString strOutput = codec->toUnicode(mKotlinProcess->readAll());
    if(strOutput.endsWith(LineSep))
    {
        strOutput.chop(LineSep.length());
    }
    mOutputWidget->append(strOutput);
    QTextCursor cursor = mOutputWidget->textCursor();
    cursor.movePosition(QTextCursor::End);
    mOutputWidget->setTextCursor(cursor);
}

void MainWindow::buildError(QProcess::ProcessError error)
{
    mOutputWidget->append("Error:" + QString::number(error));
    mOutputWidget->append(mKotlinProcess->errorString());
}

void MainWindow::slotOpenFile(bool)
{
    QAction *act = qobject_cast<QAction *>(sender());
    if(act)
    {
        QString fileName = act->data().toString();
        int idx = findEditorIndex(fileName);
        if(idx >= 0)
        {
            mCodeTabWidget->setCurrentIndex(idx);
        }
        else
        {
            QString file = mCurProjectPath + "/" + fileName;
            openFile(file);
        }
    }
}

void MainWindow::slotAddFileToProject(bool)
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Browser"), QString());
    foreach (QString file, fileList) {
        openFile(file);
        addFileToProject(file);

        QTreeWidgetItem *rootItem = mProjectTreeWidget->topLevelItem(0);
        if(rootItem)
        {
            QFile pFile(mCurProjectPath + "/" + rootItem->text(0) + ProjectSuffix);
            if(pFile.open(QIODevice::Append))
            {
                QByteArray datagram;
                datagram.append(file);
                datagram.append(LineSep);
                pFile.write(datagram);
                pFile.close();
            }
        }
    }
}

void MainWindow::slotCreateNewFileOfProject(bool)
{
    QString newFile = QInputDialog::getText(this, tr("New file"), tr("File name:"), QLineEdit::Normal, "file.kt");
    if(newFile.isEmpty())
    {
        return;
    }
    if(!newFile.endsWith(".kt"))
    {
        newFile.append(".kt");
    }
    newFile = mCurProjectPath + "/" + newFile;
    QFile pFile(newFile);
    if(pFile.open(QIODevice::WriteOnly))
    {
        pFile.close();
    }
    addFileToProject(newFile);
    openFile(newFile);

    QTreeWidgetItem *rootItem = mProjectTreeWidget->topLevelItem(0);
    if(rootItem)
    {
        QFile pFile(mCurProjectPath + "/" + rootItem->text(0) + ProjectSuffix);
        if(pFile.open(QIODevice::Append))
        {
            QByteArray datagram;
            datagram.append(newFile);
            datagram.append(LineSep);
            pFile.write(datagram);
            pFile.close();
        }
    }
}

void MainWindow::setupUI()
{
    mProjectTreeWidget = new QTreeWidget;
    mCodeTabWidget = new QTabWidget;
    mOutputWidget = new QTextEdit;
    mProjectTreeWidget->header()->setVisible(false);
    mProjectTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    mCodeTabWidget->setStyleSheet("background-color: #F0F0F0;");
    mCodeTabWidget->setTabsClosable(true);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, 0);
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, 0);
    mainSplitter->setHandleWidth(0);
    rightSplitter->setHandleWidth(0);
    rightSplitter->addWidget(mCodeTabWidget);
    rightSplitter->addWidget(mOutputWidget);
    rightSplitter->setStretchFactor(0, 1);

    mainSplitter->addWidget(mProjectTreeWidget);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(1, 1);

    setCentralWidget(mainSplitter);
}

void MainWindow::initConnect()
{
    mKotlinProcess = new QProcess;
    mKotlinProcess->setReadChannel(QProcess::StandardOutput);
    connect(mKotlinProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readOutput);
    connect(mKotlinProcess, &QProcess::errorOccurred, this, &MainWindow::buildError);
    connect(mProjectTreeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::projectTreeContextMenu);
    connect(mProjectTreeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::projectFileDoubleClicked);
    connect(mCodeTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeFile);
}

void MainWindow::on_actionNew_Project_triggered()
{
    QString prjFile = QFileDialog::getSaveFileName(this, tr("New project"), QString(), tr("Project files (*%1)").arg(ProjectSuffix));
    if(prjFile.isEmpty())
    {
        return;
    }
    if(!prjFile.endsWith(ProjectSuffix))
    {
        prjFile.append(ProjectSuffix);
    }

    newProject(prjFile);
}

void MainWindow::on_actionOpen_Project_triggered()
{
    QString prjFile = QFileDialog::getOpenFileName(this, tr("Open project"), QString(), tr("Project files (*%1)").arg(ProjectSuffix));
    if(prjFile.isEmpty())
    {
        return;
    }
    openProject(prjFile);
}

void MainWindow::on_actionBuild_triggered()
{
    QTreeWidgetItem *rootItem = mProjectTreeWidget->topLevelItem(0);
    if(rootItem)
    {
        QFile pFile(mCurProjectPath + "/" + rootItem->text(0) + ProjectSuffix);
        if(pFile.open(QIODevice::ReadOnly))
        {
            QString content = pFile.readAll();
            pFile.close();
            QStringList fileList = content.split(LineSep, QString::SkipEmptyParts);
            foreach (QString file, fileList) {
                QFileInfo info(file);
                QString kotlinPath = "C:\\kotlinc\\bin\\kotlinc";
                file.replace("/", "\\");
                QString target = info.absolutePath() + "/" + info.baseName() + ".jar";
                target.replace("/", "\\");
                QString arg = QString("/c %1 %2 -include-runtime -d %3").arg(kotlinPath).arg(file).arg(target);
                qDebug() << arg;
                mKotlinProcess->start("cmd.exe", QStringList() << arg);
                mKotlinProcess->waitForFinished();
            }
        }
    }
}

void MainWindow::on_actionRun_triggered()
{
    QTreeWidgetItem *rootItem = mProjectTreeWidget->topLevelItem(0);
    if(rootItem)
    {
        QFile pFile(mCurProjectPath + "/" + rootItem->text(0) + ProjectSuffix);
        if(pFile.open(QIODevice::ReadOnly))
        {
            QString content = pFile.readAll();
            pFile.close();

            QString mainFile;
            QStringList fileList = content.split(LineSep, QString::SkipEmptyParts);
            foreach (QString file, fileList) {
                QFile codeFile(file);
                if(codeFile.open(QIODevice::ReadOnly))
                {
                    QString content = codeFile.readAll();
                    codeFile.close();

                    QRegExp rex("fun\\s*main\\s*\\(\\s*args\\s*:\\s*Array\\s*<\\s*String\\s*>\\s*\\)");
                    rex.setMinimal(true);
                    if(content.contains(rex))
                    {
                        mainFile = file;
                        break;
                    }
                }
            }

            QFileInfo info(mainFile);
            if(info.exists())
            {
                QString jarFile = info.absolutePath() + "/" + info.baseName() + ".jar";
                info.setFile(jarFile);
                if(info.exists())
                {
                    mKotlinProcess->start("java", QStringList() << "-jar" << jarFile);
                }
            }
        }
    }
}
