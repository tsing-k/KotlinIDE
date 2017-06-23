#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CodeEditor *textEdit = new CodeEditor;
    setCentralWidget(textEdit);
}

MainWindow::~MainWindow()
{
    delete ui;
}
