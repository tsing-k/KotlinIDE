#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
  , mProjectTreeWidget(0)
  , mCodeTabWidget(0)
{
    ui->setupUi(this);

    mProjectTreeWidget = new QTreeWidget;
    mCodeTabWidget = new QTabWidget;
    mProjectTreeWidget->header()->setVisible(false);
    mCodeTabWidget->setStyleSheet("background-color: #F0F0F0;");
    mCodeTabWidget->setTabsClosable(true);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, 0);
    QSplitter *rightSplitter = new QSplitter;
    mainSplitter->setHandleWidth(0);
    rightSplitter->addWidget(mCodeTabWidget);

    mainSplitter->addWidget(mProjectTreeWidget);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(1, 1);

    setCentralWidget(mainSplitter);

    slotNewFile("New File");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotNewFile(const QString &str)
{
    mCodeTabWidget->addTab(new CodeEditor(), str);
}
