#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "video_thread.h"  // VideoThread 클래스 헤더

#include <QStackedWidget>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QScrollArea>

#include "networkmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void sendStartRequest();

signals:
    void imageLoaded(const QString &imagePath);

private slots:
    void updateFrame(const QImage &frame);
    void showChartPage();
    void showImagePage();
    void onFunctionStartButtonClicked();
    void onFunctionStopButtonClicked();
    void sendResumeRequest();
    void sendStopRequest();
    void sendRewindRequest();
    void onExitButtonClicked();
    void onRequestFinished();
    void onErrorOccurred(QNetworkReply::NetworkError error);
    void onDetectionStartButtonClicked();
    void updateImageScrollArea();
    
    void processParsedXML(const QString &description, const QString &timestamp);
    void addImageToScrollArea(const QString &imagePath);

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QScrollArea *imageScrollArea;
    QChartView *chartView;
    QNetworkAccessManager *networkAccessManager;
    VideoThread *videoThread;
    NetworkManager *networkManager;

    enum Mode { None, Bestshot, Detection } currentMode = None;

    QByteArray buffer;

    QString saveImageToFile(const QByteArray &imageData);
    void sendNetworkRequest(const QUrl &url);
};

#endif // MAINWINDOW_H