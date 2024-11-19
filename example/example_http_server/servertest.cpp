#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// 전역 변수
std::atomic<bool> is_streaming(false);   // 스트리밍 상태 플래그
std::atomic<bool> stop_requested(false); // 중단 요청 플래그
std::mutex frame_mutex;                  // 현재 프레임 보호
cv::Mat current_frame;                   // 현재 재생 중인 프레임

// 비디오 재생 스레드
void video_processing_thread(const std::string& video_path) {
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return;
    }

    while (!stop_requested) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "End of video. Restarting..." << std::endl;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0); // 영상 처음으로 돌아가기
            continue;
        }

        // 현재 프레임 저장
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            current_frame = frame.clone();
        }

        // 30fps 속도를 유지
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    cap.release();
}

// 스트리밍 스레드
void streaming_thread(tcp::socket socket) {
    try {
        // HTTP 응답 헤더 전송
        std::string header =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
        boost::asio::write(socket, net::buffer(header));

        // 스트리밍 루프
        while (is_streaming && !stop_requested) {
            cv::Mat frame;

            // 현재 프레임 가져오기
            {
                std::lock_guard<std::mutex> lock(frame_mutex);
                if (current_frame.empty()) {
                    std::cerr << "Warning: No frame available yet." << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                frame = current_frame.clone();
            }

            // JPEG로 인코딩
            std::vector<uchar> buffer;
            std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
            cv::imencode(".jpg", frame, buffer);

            // HTTP 멀티파트 형식 작성
            std::string body = "--frame\r\n"
                               "Content-Type: image/jpeg\r\n\r\n";
            body.append(buffer.begin(), buffer.end());
            body += "\r\n";

            // 이미지 전송
            beast::error_code ec;
            boost::asio::write(socket, net::buffer(body), ec);

            if (ec) {
                std::cerr << "Error sending image: " << ec.message() << std::endl;
                break;
            }

            // 5초 대기
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        // 스트리밍 종료
        std::string end_boundary = "--frame--\r\n";
        boost::asio::write(socket, net::buffer(end_boundary));

    } catch (std::exception& e) {
        std::cerr << "Exception in streaming_thread: " << e.what() << std::endl;
    }

    is_streaming = false; // 스트리밍 상태 해제
}

// 요청 처리
void handle_request(http::request<http::string_body> const& req, tcp::socket socket) {
    if (req.target() == "/start") {
        if (is_streaming) {
            std::string msg = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/plain\r\n\r\n"
                              "Streaming already started.";
            boost::asio::write(socket, net::buffer(msg));
            return;
        }

        is_streaming = true;
        stop_requested = false;

        // 새로운 스레드에서 스트리밍 시작
        std::thread(streaming_thread, std::move(socket)).detach();
    } else if (req.target() == "/stop") {
        stop_requested = true;

        std::string msg = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n\r\n"
                          "Streaming stopped.";
        boost::asio::write(socket, net::buffer(msg));
    } else {
        std::string msg = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: text/plain\r\n\r\n"
                          "Invalid endpoint.";
        boost::asio::write(socket, net::buffer(msg));
    }
}

// HTTP 서버
void http_server(unsigned short port) {
    try {
        net::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));

        for (;;) {
            tcp::socket socket(ioc);
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            handle_request(req, std::move(socket));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in http_server: " << e.what() << std::endl;
    }
}

// 메인 함수
int main() {
    try {
        unsigned short port = 5000;
        std::string video_path = "HDCCTVTraffic.mp4";

        std::cout << "HTTP Server is running on http://127.0.0.1:" << port << std::endl;

        // 비디오 재생 스레드 시작
        std::thread video_thread(video_processing_thread, video_path);

        // HTTP 서버 실행
        http_server(port);

        // 종료 처리
        video_thread.join();
    } catch (std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

