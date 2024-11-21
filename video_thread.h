#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class VideoThread : public QThread
{
    Q_OBJECT

public:
    explicit VideoThread(QObject *parent = nullptr);
    ~VideoThread();

    void stopPipeline();

protected:
    void run() override;

signals:
    void frameCaptured(const QImage &frame);

private:
    cv::VideoCapture *cap = nullptr;
};

#endif // VIDEO_THREAD_H
