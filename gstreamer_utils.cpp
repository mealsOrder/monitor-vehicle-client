#include "gstreamer_utils.h"
#include <gst/gst.h>
#include <QDebug>

static bool gstreamerInitialized = false;

void initializeGStreamer() {
    if (!gstreamerInitialized) {
        gst_init(nullptr, nullptr);
        gstreamerInitialized = true;
        qDebug() << "GStreamer initialized.";
    }
}

void deinitializeGStreamer() {
    if (gstreamerInitialized) {
        gst_deinit();
        gstreamerInitialized = false;
        qDebug() << "GStreamer deinitialized.";
    }
}
