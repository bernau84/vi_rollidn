#-------------------------------------------------
#
# Project created by QtCreator 2015-10-09T08:17:16
#
#-------------------------------------------------

QT       += core

CONFIG   += console
CONFIG   -= app_bundle

TARGET = processing
TEMPLATE = app

SOURCES += main.cpp

HEADERS  += \
    i_proc_stage.h \
    ../t_vi_setup.h \
    t_vi_proc_colortransf.h \
    t_vi_proc_threshold.h \
    t_vi_proc_contours.h

INCLUDEPATH += "C:\\opencv\\build\\include"

LIBS += -L"C:\\opencv\\build\\x86\\vc10\\lib" \
    -lopencv_core2410d \
    -lopencv_highgui2410d \
    -lopencv_imgproc2410d \
    -lopencv_features2d2410d \
    -lopencv_calib3d2410d

