#include "video_thread.h"
#include <QDebug>
#include <gst/gst.h>

VideoThread::VideoThread(QObject *parent) : QThread(parent), cap(nullptr) {}

VideoThread::~VideoThread()
{
    stopPipeline();
}

void VideoThread::run()
{
    try {
        std::string pipeline = "udpsrc port=5004 caps=\"application/x-rtp, payload=96\" ! "
                               "rtpjpegdepay ! queue ! jpegdec ! queue ! "
                               "videoconvert ! appsink";

        cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
        if (!cap.isOpened()) {
            throw std::runtime_error("Error: Could not open GStreamer pipeline.");
        }

        cv::Mat frame;
        while (!isInterruptionRequested()) {
            if (!cap.read(frame)) {
                qDebug() << "Error: Could not grab frame.";
                break;
            }

            if (frame.empty()) {
                qDebug() << "Error: Frame is empty.";
                break;
            }

            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            emit frameCaptured(qimg);

            QThread::msleep(30);
        }

        cap.release();
        qDebug() << "VideoThread exiting gracefully.";

    } catch (const std::exception &e) {
        qDebug() << "Exception occurred: " << e.what();
    }
}



void VideoThread::stopPipeline()
{
    if (cap) {
        qDebug() << "Releasing GStreamer pipeline...";
        cap->release(); // GStreamer 파이프라인 해제
        delete cap;     // 메모리 해제
        cap = nullptr;
        qDebug() << "GStreamer pipeline stopped.";
    }
    // GStreamer 전체 시스템 종료
    gst_deinit();
    qDebug() << "GStreamer deinitialized.";
}


