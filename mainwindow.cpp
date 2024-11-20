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

    // QStackedWidget�� ������ �߰�
    stackedWidget->addWidget(chartView);        // ù ��° ������
    stackedWidget->addWidget(imageScrollArea);  // �� ��° ������

    // ��ư�� ������ ��ȯ ����
    connect(ui->graph_button, &QPushButton::clicked, this, &MainWindow::showChartPage);
    connect(ui->bestshot_button, &QPushButton::clicked, this, &MainWindow::showImagePage);

    connect(ui->video_start_button, &QPushButton::clicked, this, &MainWindow::sendResumeRequest);
    connect(ui->video_stop_button, &QPushButton::clicked, this, &MainWindow::sendStopRequest);
    connect(ui->video_resume_button, &QPushButton::clicked, this, &MainWindow::sendRewindRequest);

    connect(networkManager, &NetworkManager::requestFinished, this, &MainWindow::onRequestFinished);
    connect(networkManager, &NetworkManager::errorOccurred, this, &MainWindow::onErrorOccurred);

    connect(ui->exit_button, &QPushButton::clicked, this, &MainWindow::onExitButtonClicked);

    connect(ui->detection_button, &QPushButton::clicked, this, &MainWindow::onDetectionStartButtonClicked);

    connect(ui->func_start_button, &QPushButton::clicked, this, &MainWindow::onFunctionStartButtonClicked);
    connect(ui->func_stop_button, &QPushButton::clicked, this, &MainWindow::onFunctionStopButtonClicked);

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
    currentMode = Bestshot;
    qDebug() << "Mode switched to Bestshot.";

    QString directoryPath = QDir::currentPath() + "/bestshot_images";
    QDir directory(directoryPath);
    QStringList imageFiles = directory.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp", QDir::Files);

    QWidget *containerWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(containerWidget);

    for (const QString &fileName : imageFiles) {
        QString imagePath = directoryPath + "/" + fileName;

        QLabel *imageLabel = new QLabel();
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.baseName();

            QLabel *nameLabel = new QLabel(baseName);
            nameLabel->setAlignment(Qt::AlignCenter);
            nameLabel->setStyleSheet("color: white; font-size: 14px;");

            QVBoxLayout *imageLayout = new QVBoxLayout();
            imageLayout->addWidget(imageLabel);
            imageLayout->addWidget(nameLabel);

            QWidget *imageContainer = new QWidget();
            imageContainer->setLayout(imageLayout);
            layout->addWidget(imageContainer);
        } else {
            qWarning() << "Failed to load image:" << imagePath;
        }
    }

    containerWidget->setLayout(layout);
    imageScrollArea->setWidget(containerWidget);

    updateImageScrollArea();

    stackedWidget->setCurrentWidget(imageScrollArea);

}

void MainWindow::sendStartRequest()
{
    QUrl url("http://192.168.10.121:8080/start_stream");  // START ��û URL
    qDebug() << "Sending START request to:" << url.toString();
    sendNetworkRequest(url);
}

void MainWindow::sendResumeRequest()
{
    QUrl url("http://192.168.10.121:8080/resume_stream");  // START ��û URL
    qDebug() << "Sending RESUME request to:" << url.toString();
    sendNetworkRequest(url);
}

void MainWindow::sendStopRequest()
{
    QUrl url("http://192.168.10.121:8080/pause_stream");  // STOP ��û URL
    qDebug() << "Sending STOP request to:" << url.toString();
    sendNetworkRequest(url);
}

void MainWindow::sendRewindRequest()
{
    QUrl url("http://192.168.10.121:8080/rewind_stream");  // RESUME ��û URL
    qDebug() << "Sending REWIND request to:" << url.toString();
    sendNetworkRequest(url);
}

void MainWindow::sendNetworkRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    qDebug() << "Sending GET request to:" << url.toString();
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

void MainWindow::onExitButtonClicked() {
    qDebug() << "Exit button clicked.";
}

void MainWindow::saveImageToFile(const QByteArray &imageData)
{
    // 저장할 디렉터리
    QString savePath = QCoreApplication::applicationDirPath() + "/bestshot_images";
    QDir dir(savePath);

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory:" << savePath;
            return;
        }
    }

    // 고유 파일 이름 생성
    static int imageIndex = 0;
    QString fileName = QString("image_%1.jpg").arg(imageIndex++, 3, 10, QChar('0'));
    QString filePath = dir.filePath(fileName);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        qDebug() << "Image saved to:" << filePath;
    } else {
        qWarning() << "Failed to save image:" << filePath;
    }
}



void MainWindow::updateImageScrollArea()
{
    // 이미지와 텍스트 정보를 내부적으로 처리
    QString directoryPath = QCoreApplication::applicationDirPath() + "/bestshot_images";
    QDir directory(directoryPath);
    QStringList imageFiles = directory.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.png", QDir::Files);

    // 스크롤 영역 위젯 확인 (초기화되지 않았으면 새로 생성)
    QWidget *containerWidget = imageScrollArea->widget();
    QVBoxLayout *mainLayout = nullptr;

    if (!containerWidget) {
        containerWidget = new QWidget();
        mainLayout = new QVBoxLayout(containerWidget);
        containerWidget->setLayout(mainLayout);
        imageScrollArea->setWidget(containerWidget);
    } else {
        mainLayout = qobject_cast<QVBoxLayout *>(containerWidget->layout());
    }

    // 기존 레이아웃 초기화 (리셋)
    QLayoutItem *child;
    while ((child = mainLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 이미지 파일을 읽고 UI 항목 생성
    for (const QString &fileName : imageFiles) {
        QString imagePath = directoryPath + "/" + fileName;

        // 개별 항목 위젯 생성
        QWidget *itemWidget = new QWidget();
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);

        // 왼쪽: 이미지
        QLabel *imageLabel = new QLabel();
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imageLabel->setFixedSize(150, 150);
        } else {
            imageLabel->setText("No Image");
            imageLabel->setAlignment(Qt::AlignCenter);
            imageLabel->setFixedSize(150, 150);
            imageLabel->setStyleSheet("color: white; background-color: #444;");
        }

        // 오른쪽: 텍스트 정보
        QVBoxLayout *textLayout = new QVBoxLayout();

        QFileInfo fileInfo(fileName);
        QString baseName = fileInfo.baseName(); // 파일 이름

        QLabel *licenseLabel = new QLabel("License: " + baseName);
        licenseLabel->setStyleSheet("font-size: 14px; color: white;");

        QLabel *colorLabel = new QLabel("Color: Unknown"); // 예시: 기본값
        colorLabel->setStyleSheet("font-size: 14px; color: white;");

        QLabel *speedLabel = new QLabel("Speed: Unknown"); // 예시: 기본값
        speedLabel->setStyleSheet("font-size: 14px; color: white;");

        // 텍스트 레이아웃에 추가
        textLayout->addWidget(licenseLabel);
        textLayout->addWidget(colorLabel);
        textLayout->addWidget(speedLabel);
        textLayout->addStretch(); // 남은 공간 채우기

        // 이미지와 텍스트를 레이아웃에 추가
        itemLayout->addWidget(imageLabel);
        itemLayout->addLayout(textLayout);

        // 스타일 지정
        itemWidget->setStyleSheet("border: 1px solid white; background-color: #333; margin: 5px; border-radius: 8px;");
        itemWidget->setFixedHeight(160);

        // 메인 레이아웃에 항목 추가
        mainLayout->addWidget(itemWidget);
    }

    // 스크롤 영역 업데이트
    imageScrollArea->setWidgetResizable(true);
}

void MainWindow::onDetectionStartButtonClicked(){
    currentMode = Detection;
    qDebug() << "Mode switched to Detection.";
}

void MainWindow::onFunctionStartButtonClicked()
{
    if (currentMode == Bestshot) {
        qDebug() << "Starting Bestshot mode...";
        QUrl url("http://127.0.0.1:5000/start");
        qDebug() << "Sending START request to:" << url.toString();

        QNetworkRequest request(url);
        QNetworkReply *reply = networkAccessManager->get(request);

        // ��Ʈ���� ������ ó��
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            static QByteArray buffer;
            buffer.append(reply->readAll());

            QByteArray boundary = QByteArrayLiteral("--frame");
            while (buffer.contains(boundary)) {
                int boundaryIndex = buffer.indexOf(boundary);
                int nextBoundaryIndex = buffer.indexOf(boundary, boundaryIndex + boundary.size());

                if (nextBoundaryIndex == -1) {
                    // �����Ͱ� ���� �� ������ ����
                    break;
                }

                // ��Ƽ��Ʈ ������ ����
                QByteArray part = buffer.mid(boundaryIndex + boundary.size(), nextBoundaryIndex - boundaryIndex - boundary.size());
                buffer.remove(0, nextBoundaryIndex);

                // �̹��� ������ ����
                if (part.contains("Content-Type: image/jpeg")) {
                    int imageStart = part.indexOf("\r\n\r\n") + 4; // ���� ��
                    if (imageStart > 3) {
                        QByteArray imageData = part.mid(imageStart);

                        // ���Ϸ� ����
                        saveImageToFile(imageData);

                        // UI ������Ʈ
                        updateImageScrollArea();
                    }
                }
            }
        });

        // ���� ó��
        connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
            qWarning() << "Network error:" << code;
        });

        // ��Ʈ���� ���� ó��
        connect(reply, &QNetworkReply::finished, this, [reply]() {
            qDebug() << "Streaming finished.";
            reply->deleteLater();
        });
    } else if (currentMode == Detection) {
        qDebug() << "Starting Detection mode...";
        sendNetworkRequest(QUrl("http://192.168.10.121:8080/start_detection"));
    } else {
        qWarning() << "No mode selected. Please choose a mode first.";
    }
}


void MainWindow::onFunctionStopButtonClicked(){
    switch (currentMode) {
    case Bestshot:
        qDebug() << "Stopping Bestshot mode...";
        sendNetworkRequest(QUrl("http://127.0.0.1:5000/stop"));
        break;
    case Detection:
        qDebug() << "Stopping Detection mode...";
        sendNetworkRequest(QUrl("http://192.168.10.121:8080/pause_detection"));
        break;
    default:
        qWarning() << "No mode selected. Please choose a mode first.";
        break;
    }
}
