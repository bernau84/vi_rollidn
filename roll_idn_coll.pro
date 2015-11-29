QT += core gui
QT += widgets
QT += network

TARGET = roll_idn_coll
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

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
    js_config_collection_all.txt

HEADERS += \
    cameras/i_camera_base.h \
    cameras/t_camera_attrib.h \
    cinterface/i_comm_base.h \
    cinterface/t_comm_parser.h \
    processing/i_proc_stage.h \
    processing/t_vi_proc_roi_grayscale.h \
    processing/t_vi_proc_roll_approx.h \
    processing/t_vi_proc_threshold_cont.h \
    t_vi_setup.h \
    cameras/basler/t_vi_camera_basler_usb.h \
    cameras/offline/t_vi_camera_offline_file.h \
    cinterface/cmd_line/t_vi_comm_std_terminal.h \
    cinterface/tcp_client/t_vi_comm_tcp_uni.h \
    t_roll_idn_collection.h \
    t_roll_idn_record_storage.h

DEFINES += USE_USB

RESOURCES += \
    processing/defaults.qrc
