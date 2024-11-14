#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoThread(new VideoThread(this))
{
    ui->setupUi(this);

    // QLabel의 크기를 640x480으로 고정
    ui->videoLabel->setFixedSize(800, 600);

    // VideoThread에서 frameCaptured 신호를 받아서 QLabel에 표시
    connect(videoThread, &VideoThread::frameCaptured, this, &MainWindow::updateFrame);

    // 스레드 시작
    videoThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;

    videoThread->quit();
    videoThread->wait();
}

void MainWindow::updateFrame(const QImage &frame)
{
    // QImage로 받은 프레임을 640x480 크기로 리사이즈
    QImage resizedFrame = frame.scaled(800, 600, Qt::KeepAspectRatio);

    // QLabel에 리사이즈된 프레임 표시
    ui->videoLabel->setPixmap(QPixmap::fromImage(resizedFrame));
}

