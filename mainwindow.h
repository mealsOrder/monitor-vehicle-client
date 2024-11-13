#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QThread>
#include <QPushButton>  // QPushButton 헤더 추가
#include <QHBoxLayout>  // QHBoxLayout 헤더 추가
#include "video_thread.h"  // VideoThread 클래스 헤더

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame(const QImage &frame);  // 프레임을 업데이트하는 슬롯

private:
    Ui::MainWindow *ui;
    VideoThread *videoThread;  // VideoThread 객체
    QLabel *videoLabel;        // 비디오를 표시할 QLabel
    QPushButton *dummyButton;  // 기능 없는 버튼 추가
};
#endif // MAINWINDOW_H
