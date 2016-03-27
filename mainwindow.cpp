#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::measured(QImage &im, double length, double diameter){

    QSize m(ui->canvas->width(), ui->canvas->height());
    QImage rescaled = im.scaled(m);
    ui->canvas->setPixmap(QPixmap::fromImage(rescaled));

    ui->lineEdit->setText(QString::number(length));
    ui->lineEdit_2->setText(QString::number(diameter));
}

void MainWindow::preview(QImage &im, double length, double diameter){

    QSize m(ui->preview->width(), ui->preview->height());
    QImage rescaled = im.scaled(m);
    ui->preview->setPixmap(QPixmap::fromImage(rescaled));

    ui->lineEdit->setText(QString::number(length));
    ui->lineEdit_2->setText(QString::number(diameter));
}
