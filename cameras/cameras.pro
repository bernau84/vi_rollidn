#-------------------------------------------------
#
# Project created by QtCreator 2015-10-07T18:45:32
#
#-------------------------------------------------

QT       += core gui
QT       += widgets

TARGET = cameras

TEMPLATE = app

INCLUDEPATH += "c:\Program Files\Basler\pylon 4\pylon\include"
INCLUDEPATH += "c:\Program Files\Basler\pylon 4\genicam\library\CPP\include"

LIBS += -L"c:\Program Files\Basler\pylon 4\pylon\lib\Win32"
LIBS += -L"c:\Program Files\Basler\pylon 4\genicam\library\CPP\Lib\Win32_i86"

SOURCES += main.cpp

HEADERS += \
    i_camera_base.h \
    t_camera_attrib.h \
    ../t_vi_setup.h \
    basler/t_vi_camera_basler_usb.h \
    offline/t_vi_camera_offline_file.h

DISTFILES += \
    js_camera_base.txt


