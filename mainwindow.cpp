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
#include <QDomDocument>

// 서버 및 경로 상수
const QString SERVER_IP = "192.168.10.121";
const QString SERVER_PORT = "8080";
const QString START_STREAM_PATH = "/start_stream";
const QString PAUSE_STREAM_PATH = "/pause_stream";
const QString RESUME_STREAM_PATH = "/resume_stream";
const QString REWIND_STREAM_PATH = "/rewind_stream";
const QString PREPROCESS_DETECTION_PATH = "/preprocess_detection";

// UI 및 레이아웃 상수
const int IMAGE_WIDTH = 150;
const int IMAGE_HEIGHT = 150;
const int ITEM_HEIGHT = 160;
const QString IMAGE_BORDER_COLOR = "#333";
const QString TEXT_COLOR = "white";
const QString FONT_SIZE = "14px";
const int MAX_BUFFER_SIZE = 10 * 1024 * 1024; // 10MB

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

    connect(this, &MainWindow::imageLoaded, this, &MainWindow::addImageToScrollArea);
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

QString MainWindow::buildUrl(const QString &path) {
    return QString("http://%1:%2%3").arg(SERVER_IP).arg(SERVER_PORT).arg(path);
}

void MainWindow::manageBufferSize(QByteArray &buffer) {
    if (buffer.size() > MAX_BUFFER_SIZE) {
        qWarning() << "Buffer exceeded limit. Clearing old data.";
        buffer = buffer.right(MAX_BUFFER_SIZE / 2);
    }
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
    QString directoryPath = QCoreApplication::applicationDirPath() + "/bestshot_images";
    QDir directory(directoryPath);
    QStringList imageFiles = directory.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.png", QDir::Files);

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

    QLayoutItem *child;
    while ((child = mainLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    for (const QString &fileName : imageFiles) {
        QString imagePath = directoryPath + "/" + fileName;
        addImageToScrollArea(imagePath);
    }

    imageScrollArea->setWidgetResizable(true);
    qDebug() << "ImageScrollArea updated with" << imageFiles.size() << "images.";
}

void MainWindow::addImageToScrollArea(const QString &imagePath)
{
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

    QWidget *itemWidget = new QWidget();
    QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);

    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(imagePath);
    imageLabel->setPixmap(pixmap.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);

    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *licenseLabel = new QLabel("License: " + QFileInfo(imagePath).baseName());
    licenseLabel->setStyleSheet(QString("font-size: %1; color: %2;").arg(FONT_SIZE, TEXT_COLOR));
    textLayout->addWidget(licenseLabel);
    textLayout->addStretch();

    itemLayout->addWidget(imageLabel);
    itemLayout->addLayout(textLayout);

    itemWidget->setStyleSheet(QString("border: 1px solid white; background-color: %1; margin: 5px; border-radius: 8px;").arg(IMAGE_BORDER_COLOR));
    itemWidget->setFixedHeight(ITEM_HEIGHT);

    mainLayout->insertWidget(0, itemWidget);
}

void MainWindow::sendStartRequest()
{
    QUrl streamUrl(buildUrl(START_STREAM_PATH));
    QNetworkRequest request(streamUrl);
    QNetworkReply *reply = networkAccessManager->get(request);

    static QByteArray buffer;

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        buffer.append(reply->readAll());
        manageBufferSize(buffer);

        while (true) {
            int boundaryIndex = buffer.indexOf("--boundary");
            if (boundaryIndex == -1) break;

            int nextBoundaryIndex = buffer.indexOf("--boundary", boundaryIndex + 10);
            if (nextBoundaryIndex == -1) break;

            QByteArray part = buffer.mid(boundaryIndex + 10, nextBoundaryIndex - boundaryIndex - 10);
            buffer.remove(0, nextBoundaryIndex);

            int headerEnd = part.indexOf("\r\n\r\n");
            if (headerEnd != -1) {
                QByteArray headers = part.left(headerEnd);
                QByteArray body = part.mid(headerEnd + 4);

                if (headers.contains("Content-Type: application/xml")) {
                    QDomDocument doc;
                    if (doc.setContent(body)) {
                        QDomElement root = doc.documentElement();
                        QString description = root.firstChildElement("description").text();
                        QString timestamp = root.firstChildElement("timestamp").text();
                        processParsedXML(description, timestamp);
                    }
                } else if (headers.contains("Content-Type: image/jpeg")) {
                    int startIdx = body.indexOf("\xFF\xD8");
                    int endIdx = body.indexOf("\xFF\xD9");
                    if (startIdx != -1 && endIdx != -1) {
                        QByteArray imageData = body.mid(startIdx, endIdx - startIdx + 2);
                        QString savedPath = saveImageToFile(imageData);
                        if (!savedPath.isEmpty()) {
                            QtConcurrent::run([this, savedPath]() {
                                emit imageLoaded(savedPath);
                            });
                        }
                    }
                }
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

void MainWindow::processParsedXML(const QString &description, const QString &timestamp)
{
    qDebug() << "Processing parsed XML:";
    qDebug() << "Description:" << description;
    qDebug() << "Timestamp:" << timestamp;
}

void MainWindow::sendNetworkRequest(const QString &path)
{
    QUrl url(buildUrl(path));
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Request error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void MainWindow::sendResumeRequest()
{
    sendNetworkRequest(RESUME_STREAM_PATH);
}

void MainWindow::sendStopRequest()
{
    sendNetworkRequest(PAUSE_STREAM_PATH);
}

void MainWindow::sendRewindRequest()
{
    sendNetworkRequest(REWIND_STREAM_PATH);
}

void MainWindow::onFunctionStartButtonClicked()
{
    sendNetworkRequest(PREPROCESS_DETECTION_PATH);
}

void MainWindow::onFunctionStopButtonClicked()
{
    sendNetworkRequest(PREPROCESS_DETECTION_PATH);
}

void MainWindow::onExitButtonClicked()
{
    if (videoThread && videoThread->isRunning()) {
        videoThread->requestInterruption();
        videoThread->quit();
        videoThread->wait();
    }

    if (networkAccessManager) {
        networkAccessManager->deleteLater();
    }

    qDebug() << "Exiting application...";
    QApplication::quit();
}

void MainWindow::showImagePage()
{
    currentMode = Bestshot;
    qDebug() << "Mode switched to Bestshot.";
    updateImageScrollArea();
    stackedWidget->setCurrentWidget(imageScrollArea);
}

void MainWindow::showChartPage()
{
    if (chartView) {
        stackedWidget->setCurrentWidget(chartView);
        qDebug() << "Chart page displayed.";
    } else {
        qWarning() << "Chart view is not initialized.";
    }
}
