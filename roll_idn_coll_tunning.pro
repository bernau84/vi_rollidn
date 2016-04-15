QT += core gui
QT += widgets
QT += network

TARGET = roll_idn_coll4
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

VERSION = 2.8.4.14

SOURCES += main.cpp \
    mainwindow.cpp

INCLUDEPATH += "c:\Program Files\Basler\pylon 4\pylon\include"
INCLUDEPATH += "c:\Program Files\Basler\pylon 4\genicam\library\CPP\include"

LIBS += -L"c:\Program Files\Basler\pylon 4\pylon\lib\Win32"
LIBS += -L"c:\Program Files\Basler\pylon 4\genicam\library\CPP\Lib\Win32_i86"


INCLUDEPATH += "C:\\opencv\\build\\include"

LIBS += -L"C:\\opencv\\build\\x86\\vc10\\lib" \
    -lopencv_core2410d \
    -lopencv_highgui2410d \
    -lopencv_imgproc2410d \
    -lopencv_features2d2410d \
    -lopencv_calib3d2410d

SUBDIRS += \
    cameras/cameras.pro \
    cinterface/cinterface.pro \
    processing/processing.pro

DISTFILES += \
    cameras/js_camera_base.txt \
    processing/js_config_cylinder_approx.txt \
    processing/js_config_roi_colortransf.txt \
    processing/js_config_threshold_cont.txt \
    js_config_collection_all.txt \
    processing/js_config_rectification.txt

HEADERS += \
    cameras/i_camera_base.h \
    cameras/t_camera_attrib.h \
    t_vi_setup.h \
    cameras/basler/t_vi_camera_basler_usb.h \
    cameras/offline/t_vi_camera_offline_file.h \
    t_roll_idn_collection.h \
    t_roll_idn_record_storage.h \
    t_vi_specification.h \
    cinterface/i_comm_generic.h \
    cinterface/i_comm_parser.h \
    cinterface/t_comm_parser_binary.h \
    cinterface/t_comm_parser_string.h \
    cinterface/cmd_line/t_comm_std_terminal.h \
    cinterface/tcp_uni/t_comm_tcp_uni.h \
    mainwindow.h \
    processing/t_vi_proc_sub_background.h \
    processing/t_vi_proc_roi_colortransf.h \
    processing/t_vi_proc_rectification.h \
    processing/t_vi_proc_statistic.h \
    processing/i_proc_stage.h \
    processing/t_vi_proc_roll_approx.h \
    processing/t_vi_proc_threshold_cont.h \
    cameras/basler/t_vi_camera_basler_gige.h

DEFINES += USE_USB
#DEFINES += USE_GIGE

RESOURCES += \
    processing/defaults.qrc

FORMS += \
    mainwindow.ui

