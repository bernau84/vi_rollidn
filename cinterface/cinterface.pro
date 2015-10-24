QT += core
QT -= gui

TARGET = cinterface
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    t_vi_command_parser.h \
    i_communication_base.h \
    cmd_line/t_vi_comm_std_terminal.h \
    tcp_client/t_vi_comm_tcp_cli.h \
    ../t_vi_setup.h

