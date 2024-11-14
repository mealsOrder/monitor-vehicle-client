#include "rtpstreamworker.h"
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <iostream>

RtpStreamWorker::RtpStreamWorker(QObject *parent) : QObject(parent), cap(nullptr), isRunning(false) {}

RtpStreamWorker::~RtpStreamWorker() {
    stopStream();
}

void RtpStreamWorker::startStream() {
    if (isRunning) return;

    // GStreamer 파이프라인 설정
    std::string pipeline = "udpsrc port=5004 ! application/x-rtp, payload=96 ! "
                           "rtph264depay ! avdec_h264 ! videoconvert ! appsink";

    cap = new cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);
    if (!cap->isOpened()) {
        std::cerr << "Error: Could not open RTP stream" << std::endl;
        return;
    }

    isRunning = true;

    // 스트림을 읽어오는 루프 시작
    while (isRunning) {
        cv::Mat frame;
        (*cap) >> frame;
        if (frame.empty()) {
            std::cerr << "Warning: Empty frame received" << std::endl;
            continue;
        }

        // OpenCV Mat 이미지를 Qt QImage로 변환
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage qImg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        emit frameReady(qImg);  // 프레임이 준비되면 신호를 통해 전송

        QThread::msleep(30);  // 30ms 대기 (약 30fps)
    }

    cap->release();
    delete cap;
    cap = nullptr;
}

void RtpStreamWorker::stopStream() {
    isRunning = false;
}
