#include "mainwindow.h"
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), networkManager(new QNetworkAccessManager(this)), workerThread(new QThread(this)) {
    // 레이아웃과 UI 구성
    QVBoxLayout *layout = new QVBoxLayout(this);
    imageLabel = new QLabel("Press 'Start Stream' to begin", this);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    QPushButton *startButton = new QPushButton("Start Stream", this);
    layout->addWidget(startButton);

    connect(startButton, &QPushButton::clicked, this, &MainWindow::startStreaming);

    // RTP 스트림 작업자 설정
    worker = new RtpStreamWorker();
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &RtpStreamWorker::frameReady, this, &MainWindow::displayFrame);

    workerThread->start();
}

MainWindow::~MainWindow() {
    worker->stopStream();  // 스트림 중지
    workerThread->quit();  // 스레드 종료
    workerThread->wait();
}

void MainWindow::startStreaming() {
    // HTTP 요청 전송
    QUrl url("http://192.168.61.238:8080/start_stream");
    QNetworkRequest request(url);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::handleNetworkReply);
}

void MainWindow::handleNetworkReply() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        std::cout << "Response: " << reply->readAll().toStdString() << std::endl;
        QMetaObject::invokeMethod(worker, "startStream");  // RTP 스트림 시작
    } else {
        std::cerr << "Error: " << reply->errorString().toStdString() << std::endl;
    }
    reply->deleteLater();
}

void MainWindow::displayFrame(const QImage &frame) {
    imageLabel->setPixmap(QPixmap::fromImage(frame).scaled(imageLabel->size(), Qt::KeepAspectRatio));
}
