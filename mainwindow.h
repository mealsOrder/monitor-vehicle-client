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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void sendStartRequest();

    ~MainWindow();

private slots:
    void onRequestFinished(const QByteArray &response);  // ���� �����͸� ����
    void onErrorOccurred(const QString &errorString);    // ���� �޽����� ����

    void updateFrame(const QImage &frame);

    void showChartPage();
    void showImagePage();

    //void sendStartRequest();
    void sendStopRequest();
    void sendResumeRequest();

    void sendNetworkRequest(const QUrl &url);
    void sendRewindRequest();

    void onExitButtonClicked();

    void onBestshotStartButtonClicked();
    //void processAndSaveImages(const QByteArray &responseData,const QString& saveDirectory);

    void onBestshotStopButtonClicked();
    void saveImageToFile(const QByteArray &imageData);
    void updateImageScrollArea();

private:
    Ui::MainWindow *ui;
    VideoThread *videoThread;

    QStackedWidget *stackedWidget;
    QChartView *chartView;
    QScrollArea *imageScrollArea;

    NetworkManager *networkManager;
    QNetworkAccessManager *networkAccessManager;
};
#endif // MAINWINDOW_H

