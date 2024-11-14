#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QNetworkAccessManager>
#include "rtpstreamworker.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startStreaming();             // HTTP 요청을 보내는 슬롯
    void handleNetworkReply();         // HTTP 응답 처리 슬롯
    void displayFrame(const QImage &frame);  // 새로운 프레임을 표시하는 슬롯

private:
    QLabel *imageLabel;                // 스트림을 표시할 QLabel
    QNetworkAccessManager *networkManager;  // HTTP 요청을 위한 네트워크 관리자
    QThread *workerThread;             // RTP 스트림을 위한 작업 스레드
    RtpStreamWorker *worker;           // RTP 스트림 처리 작업자
};

#endif // MAINWINDOW_H
