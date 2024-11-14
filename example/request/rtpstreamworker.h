#ifndef RTPSTREAMWORKER_H
#define RTPSTREAMWORKER_H

#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>

class RtpStreamWorker : public QObject {
    Q_OBJECT

public:
    explicit RtpStreamWorker(QObject *parent = nullptr);
    ~RtpStreamWorker();

public slots:
    void startStream();  // RTP 스트림 시작
    void stopStream();   // RTP 스트림 중지

signals:
    void frameReady(const QImage &frame);  // 새로운 프레임이 준비되었을 때 신호 발생

private:
    cv::VideoCapture *cap;  // RTP 스트림을 위한 VideoCapture 객체
    bool isRunning;         // 스트림 실행 상태
};

#endif // RTPSTREAMWORKER_H
