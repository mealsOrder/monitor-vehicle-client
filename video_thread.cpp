#include "video_thread.h"
#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <QThread>
#include <QImage>

void VideoThread::run()
{
    // GStreamer 파이프라인
    std::string pipeline = "udpsrc port=5004 caps=\"application/x-rtp, payload=96\" ! "
                           "rtph264depay ! queue ! avdec_h264 ! queue ! "
                           "videoconvert ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open GStreamer pipeline for receiving." << std::endl;
        return;
    }

    cv::Mat frame;
    while (true) {
        // 프레임을 읽어오기
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // 프레임을 QImage로 변환하여 메인 윈도우에 전달
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        emit frameCaptured(qimg);

        // 간단한 sleep을 통해 CPU 과부하를 방지
        QThread::msleep(30);
    }

    cap.release();
}

