QT += core
QT += network
QT -= gui

TARGET = cinterface
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    cmd_line/t_vi_comm_std_terminal.h \
    tcp_client/t_vi_comm_tcp_cli.h \
    ../t_vi_setup.h \
    t_comm_parser.h \
    i_comm_base.h

