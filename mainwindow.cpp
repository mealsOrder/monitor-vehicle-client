#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <iostream>
#include <QImage>
#include <QLabel>
#include <QPushButton>  // 버튼을 추가하기 위해 필요한 헤더
#include <QHBoxLayout>  // 수평 레이아웃을 추가하기 위해 필요한 헤더
#include "video_thread.h"  // VideoThread 클래스 헤더


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoThread(new VideoThread(this))
{
    ui->setupUi(this);

    // QLabel을 사용하여 비디오 프레임을 표시
    videoLabel = new QLabel(this);

    // 버튼을 추가
    QPushButton *dummyButton = new QPushButton("Dummy Button", this);  // 기능 없는 버튼

    // 수평 레이아웃을 생성하고 QLabel과 QPushButton을 추가
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(videoLabel);  // 비디오 라벨을 레이아웃에 추가
    layout->addWidget(dummyButton);  // 버튼을 레이아웃에 추가

    // 레이아웃을 중앙 위젯에 설정
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

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
    // QImage로 받은 프레임을 QLabel에 표시
    videoLabel->setPixmap(QPixmap::fromImage(frame));
    videoLabel->resize(frame.size());
}
