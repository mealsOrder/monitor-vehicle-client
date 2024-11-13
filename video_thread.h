#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class VideoThread : public QThread
{
    Q_OBJECT

public:
    VideoThread(QObject *parent = nullptr) : QThread(parent) {}

protected:
    void run() override;

signals:
    void frameCaptured(const QImage &frame);
};

#endif // VIDEO_THREAD_H

