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
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::showChartPage);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::showImagePage);
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

