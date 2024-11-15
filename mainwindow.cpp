#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <iostream>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QDir>

#include "networkmanager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoThread(new VideoThread(this))
    , networkManager(new NetworkManager(this))
    , networkAccessManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    // QLabel의 크기를 640x480으로 고정
    ui->videoLabel->setFixedSize(800, 600);

    // VideoThread에서 frameCaptured 신호를 받아서 QLabel에 표시
    connect(videoThread, &VideoThread::frameCaptured, this, &MainWindow::updateFrame);

    // 스레드 시작
    videoThread->start();

    // QStackedWidget ���� (�����̳ʿ��� ������ stackedWidget)
    stackedWidget = ui->stackedWidget;

    // ��Ʈ�� ǥ���� ������ ����
    chartView = new QChartView(new QChart());
    chartView->chart()->setTitle("Sample Chart");
    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(1, 7);
    series->append(2, 4);
    series->append(3, 5);
    series->append(4, 10);
    chartView->chart()->addSeries(series);
    chartView->chart()->createDefaultAxes();

    // �̹��� ����Ʈ�� ǥ���� ������ ����
    imageScrollArea = new QScrollArea();
    QWidget *containerWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(containerWidget);

    // ���� �̹��� ���ϵ� �߰� (�����丮 ���� ���� �ʿ�)
    QString directoryPath = "/home/pi/ver2/hvtech/images";
    QDir directory(directoryPath);
    QStringList imageFiles = directory.entryList(QStringList() << "*.jpg" << "*.png" << "*.bmp", QDir::Files);

    for (const QString &fileName : imageFiles) {
        QString imagePath = directoryPath + "/" + fileName;
        QLabel *imageLabel = new QLabel();
        QPixmap pixmap(imagePath);
        imageLabel->setPixmap(pixmap.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        layout->addWidget(imageLabel);
    }
    containerWidget->setLayout(layout);
    imageScrollArea->setWidget(containerWidget);

    // QStackedWidget�� ������ �߰�
    stackedWidget->addWidget(chartView);        // ù ��° ������
    stackedWidget->addWidget(imageScrollArea);  // �� ��° ������

    // ��ư�� ������ ��ȯ ����
    connect(ui->graph_button, &QPushButton::clicked, this, &MainWindow::showChartPage);
    connect(ui->bestshot_button, &QPushButton::clicked, this, &MainWindow::showImagePage);

    connect(ui->video_start_button, &QPushButton::clicked, this, &MainWindow::sendStartRequest);
    connect(ui->video_stop_button, &QPushButton::clicked, this, &MainWindow::sendStopRequest);
    connect(ui->video_resume_button, &QPushButton::clicked, this, &MainWindow::sendResumeRequest);

    connect(networkManager, &NetworkManager::requestFinished, this, &MainWindow::onRequestFinished);
    connect(networkManager, &NetworkManager::errorOccurred, this, &MainWindow::onErrorOccurred);
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

void MainWindow::showChartPage()
{
    stackedWidget->setCurrentWidget(chartView);
}

void MainWindow::showImagePage()
{
    stackedWidget->setCurrentWidget(imageScrollArea);
}

void MainWindow::sendStartRequest()
{
    QUrl url("http://192.168.10.121:8080/start_stream");  // START ��û URL
    sendNetworkRequest(url);

    //networkManager->sendGetRequest(url);
    /*
    QUrl url("http://192.168.10.121:8080/start_stream");  // START ��û URL
    networkManager->sendGetRequest(url);  // NetworkManager�� �����Ͽ� GET ��û
    */
}

void MainWindow::sendStopRequest()
{
    QUrl url("http://192.168.10.121:8080/pause_stream");  // STOP ��û URL
    sendNetworkRequest(url);
}

void MainWindow::sendResumeRequest()
{
    QUrl url("http://192.168.10.121:8080/rewind_stream");  // RESUME ��û URL
    sendNetworkRequest(url);
}

void MainWindow::sendNetworkRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    //networkAccessManager->get(request);  // GET ��û
    networkManager->sendGetRequest(url);
}

void MainWindow::onRequestFinished(const QByteArray &response)
{
    qDebug() << "Request successful. Response:" << response;
}

void MainWindow::onErrorOccurred(const QString &errorString)
{
    qWarning() << "Error occurred:" << errorString;  // ���� �޽��� ����
}

