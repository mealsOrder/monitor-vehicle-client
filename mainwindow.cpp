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

    connect(ui->best_start_button, &QPushButton::clicked, this, &MainWindow::onBestshotStartButtonClicked);
    connect(ui->best_stop_button, &QPushButton::clicked, this, &MainWindow::onBestshotStopButtonClicked);
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

    stackedWidget->setCurrentWidget(imageScrollArea);

}

void MainWindow::sendStartRequest()
{
    QUrl url("http://192.168.10.121:8080/start_stream");  // START ��û URL
    qDebug() << "Sending START request to:" << url.toString();
    sendNetworkRequest(url);

    //networkManager->sendGetRequest(url);
    /*
    QUrl url("http://192.168.10.121:8080/start_stream");  // START ��û URL
    networkManager->sendGetRequest(url);  // NetworkManager�� �����Ͽ� GET ��û
    */
}

void MainWindow::sendResumeRequest()
{
    QUrl url("http://192.168.10.121:8080/resume_stream");  // START ��û URL
    qDebug() << "Sending RESUME request to:" << url.toString();
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

void MainWindow::onBestshotStartButtonClicked()
{
    QUrl url("http://127.0.0.1:5000/start");
    qDebug() << "Sending START request to:" << url.toString();

    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager->get(request);

    // 스트리밍 데이터 처리
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        static QByteArray buffer;
        buffer.append(reply->readAll());

        QByteArray boundary = QByteArrayLiteral("--frame");
        while (buffer.contains(boundary)) {
            int boundaryIndex = buffer.indexOf(boundary);
            int nextBoundaryIndex = buffer.indexOf(boundary, boundaryIndex + boundary.size());

            if (nextBoundaryIndex == -1) {
                // 다음 경계 문자열이 없다면 데이터가 아직 덜 도착한 상태
                break;
            }

            // 멀티파트 데이터 추출
            QByteArray part = buffer.mid(boundaryIndex + boundary.size(), nextBoundaryIndex - boundaryIndex - boundary.size());
            buffer.remove(0, nextBoundaryIndex);

            // 이미지 데이터 추출
            if (part.contains("Content-Type: image/jpeg")) {
                int imageStart = part.indexOf("\r\n\r\n") + 4; // 헤더 끝
                if (imageStart > 3) {
                    QByteArray imageData = part.mid(imageStart);

                    // 파일로 저장
                    saveImageToFile(imageData);

                    // UI 업데이트
                    updateImageScrollArea();
                }
            }
        }
    });

    // 에러 처리
    connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
        qWarning() << "Network error:" << code;
    });

    // 스트리밍 종료 처리
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        qDebug() << "Streaming finished.";
        reply->deleteLater();
    });
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
    QString directoryPath = QCoreApplication::applicationDirPath() + "/bestshot_images";
    QDir directory(directoryPath);
    QStringList imageFiles = directory.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.png", QDir::Files);

    // ���� ����Ʈ�� ������������ ����
    std::sort(imageFiles.begin(), imageFiles.end(), std::greater<QString>());

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
        }
    }

    containerWidget->setLayout(layout);
    imageScrollArea->setWidget(containerWidget);

    stackedWidget->setCurrentWidget(imageScrollArea);
}

/*

void MainWindow::onBestshotStartButtonClicked()
{
    QUrl url("http://127.0.0.1:5000/start");
    qDebug() << "Sending Test request to:" << url.toString();

    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Read response data
            QByteArray responseData = reply->readAll();

            // Process and save images
            QString savePath = QCoreApplication::applicationDirPath() + "/bestshot_images";
            processAndSaveImages(responseData,savePath);
        } else {
            qWarning() << "Request failed with error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
*/

/*
void MainWindow::processAndSaveImages(const QByteArray& responseData, const QString& saveDirectory) {
    QByteArray boundary("--frame");  // 경계 문자열 정의

    if (responseData.isEmpty()) {
        qWarning() << "응답 데이터가 비어 있습니다.";
        return;
    }

    // 저장할 디렉터리 생성 (존재하지 않으면 생성)
    QDir dir(saveDirectory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {  // 디렉터리 생성 시도
            qWarning() << "디렉터리를 생성할 수 없습니다:" << saveDirectory;
            return;
        }
    }

    // 데이터를 수동으로 분리
    int start = 0; // 현재 데이터 시작 위치
    int imageIndex = 0; // 이미지 파일 고유 인덱스

    while ((start = responseData.indexOf(boundary, start)) != -1) {
        // 경계 문자열 이후 데이터 확인
        int nextBoundary = responseData.indexOf(boundary, start + boundary.size());
        QByteArray part;
        if (nextBoundary != -1) {
            part = responseData.mid(start + boundary.size(), nextBoundary - start - boundary.size());
        } else {
            part = responseData.mid(start + boundary.size());
        }

        // 이미지 데이터 추출
        if (part.contains("Content-Type: image/jpeg")) {
            int imageStart = part.indexOf("\r\n\r\n") + 4; // 헤더 끝
            if (imageStart > 3 && imageStart < part.size()) {
                QByteArray imageData = part.mid(imageStart);

                // 고유 파일 경로 생성
                QString fileName = QString("image_%1.jpg").arg(imageIndex++, 3, 10, QChar('0'));
                QString filePath = dir.filePath(fileName);

                QFile file(filePath);

                // 파일 저장
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(imageData);
                    file.close();
                    qDebug() << "이미지가 저장되었습니다:" << filePath;
                } else {
                    qWarning() << "이미지 파일을 저장할 수 없습니다:" << filePath;
                }
            }
        }

        // 다음 경계를 찾기 위해 시작 위치 갱신
        start = nextBoundary;
    }
}
*/

void MainWindow::onBestshotStopButtonClicked(){

    QUrl url("http://127.0.0.1:5000/stop");  // STOP ��û URL
    qDebug() << "Sending STOP request to:" << url.toString();
    sendNetworkRequest(url);
}
