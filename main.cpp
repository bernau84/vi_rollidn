#include <QApplication>
#include <QProcessEnvironment>
#include <QDebug>
#include <QLabel>

#include <stdio.h>

#include "t_roll_idn_collection.h"
#include "cinterface\tcp_client\t_comm_tcp_uni.h"

int main(int argc, char *argv[])
{

    /*
        @Echo Off
        REM Enable pylon logging for this run. The logfile will be created in the %TEMP% directory and named pylonLog.txt
        REM You can drag-n-drop any application from the explorer onto this script file to run it with logging enabled.
        REM If you just start this script with no args the pylonViewer will be started.

        IF !%PYLON_GENICAM_VERSION%! == !! SET PYLON_GENICAM_VERSION=V2_3
        SET GENICAM_LOG_CONFIG_%PYLON_GENICAM_VERSION%=%PYLON_ROOT%\..\DebugLogging.properties

        REM start the application

        ECHO Logging activated
        ECHO Waiting for application to exit ...

        IF !%1! == !! (
            Start /WAIT "PylonViewerApp" PylonViewerApp.exe
        ) ELSE (
            Start /D"%~dp1" /WAIT "%~n1" %1
        )

        rem open an explorer window and select the logfile
        IF EXIST "%TEMP%\pylonLog.txt" Start "" explorer.exe /select,%TEMP%\pylonLog.txt
    */

    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();

    QString genicam_key = "PYLON_GENICAM_VERSION";
    QString genicam_value = "V2_3";
    qDebug() << "QProcessEnvironment Insert" << genicam_key << genicam_value;
    pe.insert(genicam_key, genicam_value);

    QString log_config_key = "GENICAM_LOG_CONFIG_";
    log_config_key += genicam_value;

    QString log_config_value = pe.value("PYLON_ROOT");
    log_config_value += pe.value("\\..\\DebugLogging.properties");

    qDebug() << "QProcessEnvironment Insert" << log_config_key << log_config_value;
    pe.insert(log_config_key, log_config_value);

    QApplication a(argc, argv);

    //QString config_path = QDir::currentPath() + "\\config.txt";
    QString config_path = "c:\\Users\\bernau84\\Documents\\sandbox\\roll_idn\\js_config_collection_all.txt";

    t_roll_idn_collection worker(config_path);
    worker.initialize();

#ifdef QT_DEBUG
    worker.on_trigger(); //start measuring
    worker.on_trigger(); //start measuring
#endif //

    return a.exec();
}

