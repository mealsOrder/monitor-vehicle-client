#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include "video_thread.h"  // VideoThread 클래스 헤더

#include <QStackedWidget>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QScrollArea>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame(const QImage &frame);

    void showChartPage();
    void showImagePage();

private:
    Ui::MainWindow *ui;
    VideoThread *videoThread;

    QStackedWidget *stackedWidget;
    QChartView *chartView;
    QScrollArea *imageScrollArea;
};

#endif // MAINWINDOW_H

