
#include <QtGui/QGuiApplication>

#include "basler/t_vi_camera_basler_usb.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    t_vi_camera_basler_usb dev;
    dev.snap(NULL, 0);

    return a.exec();
}
