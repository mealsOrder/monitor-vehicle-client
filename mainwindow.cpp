#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QNetworkReply>
#include <QScrollArea>

#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoThread(new VideoThread(this))
    , networkManager(new NetworkManager(this))
    , networkAccessManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    ui->videoLabel->setFixedSize(800, 600);

    connect(videoThread, &VideoThread::frameCaptured, this, &MainWindow::updateFrame);

    videoThread->start();

    stackedWidget = ui->stackedWidget;
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

    imageScrollArea = new QScrollArea();
    stackedWidget->addWidget(chartView);
    stackedWidget->addWidget(imageScrollArea);

    connect(ui->graph_button, &QPushButton::clicked, this, &MainWindow::showChartPage);
    connect(ui->bestshot_button, &QPushButton::clicked, this, &MainWindow::showImagePage);
    connect(ui->video_start_button, &QPushButton::clicked, this, &MainWindow::sendResumeRequest);
    connect(ui->video_stop_button, &QPushButton::clicked, this, &MainWindow::sendStopRequest);
    connect(ui->video_resume_button, &QPushButton::clicked, this, &MainWindow::sendRewindRequest);
    connect(ui->exit_button, &QPushButton::clicked, this, &MainWindow::onExitButtonClicked);
    connect(ui->detection_button, &QPushButton::clicked, this, &MainWindow::onDetectionStartButtonClicked);
    connect(ui->func_start_button, &QPushButton::clicked, this, &MainWindow::onFunctionStartButtonClicked);
    connect(ui->func_stop_button, &QPushButton::clicked, this, &MainWindow::onFunctionStopButtonClicked);
}

MainWindow::~MainWindow()
{
    if (videoThread && videoThread->isRunning()) {
        videoThread->requestInterruption();
        videoThread->quit();
        videoThread->wait();
    }
    delete videoThread;
    delete ui;
}

void MainWindow::updateFrame(const QImage &frame)
{
    QImage resizedFrame = frame.scaled(800, 600, Qt::KeepAspectRatio);
    ui->videoLabel->setPixmap(QPixmap::fromImage(resizedFrame));
}

QString MainWindow::saveImageToFile(const QByteArray &imageData) {
    if (!imageData.startsWith("\xFF\xD8") || !imageData.endsWith("\xFF\xD9")) {
        qWarning() << "Invalid JPEG data received.";
        return QString();
    }

    QString savePath = QCoreApplication::applicationDirPath() + "/bestshot_images";
    QDir dir(savePath);

    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create directory:" << savePath;
        return QString();
    }

    static int imageIndex = 0;
    QString fileName = QString("image_%1.jpg").arg(imageIndex++, 3, 10, QChar('0'));
    QString filePath = dir.filePath(fileName);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        qDebug() << "Image saved to:" << filePath;
        return filePath;
    } else {
        qWarning() << "Failed to save image:" << filePath;
        return QString();
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

    // 로그 확인
    qDebug() << "ImageScrollArea updated with" << imageFiles.size() << "images.";
}


void MainWindow::onFunctionStartButtonClicked()
{
    qDebug() << "Starting persistent stream...";

    QUrl url("http://192.168.10.121:8080/preprocess_detection");
    QNetworkRequest request(url);
    request.setRawHeader("Connection", "keep-alive"); // 연결을 유지하도록 설정
    QNetworkReply *reply = networkAccessManager->get(request);
    
/*
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        static QByteArray buffer; // 데이터를 누적하기 위한 버퍼
        buffer.append(reply->readAll()); // 수신된 데이터를 버퍼에 추가

        QByteArray boundary = QByteArrayLiteral("--frame"); // 서버에서 설정한 바운더리

        while (true) {
            // 바운더리 위치 찾기
            int boundaryIndex = buffer.indexOf(boundary);
            if (boundaryIndex == -1) break; // 바운더리가 없으면 대기

            // 다음 바운더리 위치 찾기
            int nextBoundaryIndex = buffer.indexOf(boundary, boundaryIndex + boundary.size());
            if (nextBoundaryIndex == -1) break; // 다음 바운더리가 없으면 대기

            // 바운더리 사이의 데이터 추출
            QByteArray part = buffer.mid(boundaryIndex + boundary.size(), nextBoundaryIndex - boundaryIndex - boundary.size());
            buffer.remove(0, nextBoundaryIndex); // 처리된 데이터 제거

            // JPEG 데이터 추출
            if (part.contains("Content-Type: image/jpeg")) {
                int imageStart = part.indexOf("\r\n\r\n") + 4; // 헤더를 넘어 실제 데이터 시작
                if (imageStart > 3) {
                    QByteArray imageData = part.mid(imageStart);

                    // 데이터 저장 및 UI 업데이트
                    QString savedPath = saveImageToFile(imageData);
                    if (!savedPath.isEmpty()) {
                        updateImageScrollArea();
                    }
                }
            }
        }
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
        qWarning() << "Stream error occurred:" << code;
    });

    // 스트림이 끝난 경우에도 연결이 닫히지 않도록 처리
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        qDebug() << "Stream closed by server.";
        reply->deleteLater();
    });
    */
}


void MainWindow::sendNetworkRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    networkManager->sendGetRequest(url);
}

void MainWindow::onDetectionStartButtonClicked()
{
    currentMode = Detection;
    qDebug() << "Mode switched to Detection.";
}

void MainWindow::onFunctionStopButtonClicked()
{
    if (currentMode == Bestshot) {
        sendNetworkRequest(QUrl("http://192.168.10.121:8080/preprocess_detection"));
    } else if (currentMode == Detection) {
        sendNetworkRequest(QUrl("http://192.168.10.121:8080/preprocess_detection"));
    }
}

void MainWindow::onExitButtonClicked()
{
    if (videoThread && videoThread->isRunning()) {
        videoThread->requestInterruption();
        videoThread->quit();
        videoThread->wait();
    }
    QApplication::quit();
}

void MainWindow::showImagePage()
{
    currentMode = Bestshot;
    qDebug() << "Mode switched to Bestshot.";

    updateImageScrollArea(); // 이미지를 로드하고 UI를 업데이트합니다.

    stackedWidget->setCurrentWidget(imageScrollArea); // 스크롤 영역을 현재 위젯으로 설정
}

void MainWindow::showChartPage()
{
    if (chartView) {
        stackedWidget->setCurrentWidget(chartView); // 차트 페이지를 활성화
        qDebug() << "Chart page displayed.";
    } else {
        qWarning() << "Chart view is not initialized.";
    }
}

void MainWindow::sendStartRequest() {
    QUrl streamUrl("http://192.168.10.121:8080/start_stream");
    QNetworkRequest request(streamUrl);
    QNetworkReply *reply = networkAccessManager->get(request);

    // 데이터 조립용 버퍼
    static QByteArray buffer;
    static bool imageStarted = false;

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        // 데이터를 누적
        buffer.append(reply->readAll());

        while (true) {
            if (!imageStarted) {
                // JPEG 시작 마커(FD8)를 찾음
                int startIdx = buffer.indexOf("\xFF\xD8");
                if (startIdx != -1) {
                    imageStarted = true;
                    buffer.remove(0, startIdx);  // 시작 마커 이전 데이터 제거
                } else {
                    break; // 시작 마커를 기다림
                }
            }

            // JPEG 종료 마커(FD9)를 찾음
            int endIdx = buffer.indexOf("\xFF\xD9");
            if (imageStarted && endIdx != -1) {
                QByteArray imageData = buffer.left(endIdx + 2);  // FFD9 포함하여 추출
                buffer.remove(0, endIdx + 2);                   // 조립된 데이터 제거

                imageStarted = false;  // 다음 이미지를 위해 초기화

                // 이미지를 파일로 저장 및 UI 업데이트
                QString savedPath = saveImageToFile(imageData);
                if (!savedPath.isEmpty()) {
                    updateImageScrollArea();
                }
            } else {
                break; // 데이터가 부족하면 대기
            }
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Stream error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}


void MainWindow::sendResumeRequest()
{
    sendNetworkRequest(QUrl("http://192.168.10.121:8080/resume_stream"));
}

void MainWindow::sendStopRequest()
{
    sendNetworkRequest(QUrl("http://192.168.10.121:8080/pause_stream"));
}

void MainWindow::sendRewindRequest()
{
    sendNetworkRequest(QUrl("http://192.168.10.121:8080/rewind_stream"));
}

void MainWindow::onRequestFinished()
{
    // sender()를 사용하여 QNetworkReply 가져오기
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        qWarning() << "Reply is null";
        return;
    }

    QByteArray response = reply->readAll();
    qDebug() << "Request finished with response:" << response.left(100); // 응답 로그 확인
    reply->deleteLater(); // 응답 객체 해제
}

void MainWindow::onErrorOccurred(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        qWarning() << "Reply is null on error";
        return;
    }

    qWarning() << "Network Error occurred:" << error;
    reply->deleteLater(); // 응답 객체 해제
}