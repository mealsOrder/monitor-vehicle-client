QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# OpenCV 설정 (pkg-config 방식)
PKGCONFIG = pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv4


# GStreamer 헤더 파일 경로 추가
INCLUDEPATH += /usr/include/gstreamer-1.0

# GLib 헤더 파일 경로 추가 (만약 GLib이 다른 경로에 있다면 해당 경로를 추가)
INCLUDEPATH += /usr/include/glib-2.0
INCLUDEPATH += /usr/lib/aarch64-linux-gnu/glib-2.0/include

# GStreamer 라이브러리 경로 추가
LIBS += -L/usr/lib/aarch64-linux-gnu/gstreamer-1.0 -lgstreamer-1.0

# GLib 라이브러리 경로 추가 (필요한 경우)
LIBS += -L/usr/lib/aarch64-linux-gnu -lglib-2.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    video_thread.cpp

HEADERS += \
    mainwindow.h \
    video_thread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
