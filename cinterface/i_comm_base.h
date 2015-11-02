#ifndef I_COMMUNICATION_BASE
#define I_COMMUNICATION_BASE

#include <stdio.h>
#include <stdint.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QWaitCondition>

#include "t_comm_parser.h"

#define VI_COMM_REFRESH_RT  50  //definuje reakcni cas

class i_vi_comm_base : public QObject,  t_vi_comm_parser {

    Q_OBJECT

protected:
    enum e_commsta {

        COMMSTA_UNKNOWN = 0,
        COMMSTA_PREPARED,
        COMMSTA_INPROC,
        COMMSTA_ERROR
    } sta;

public:

    virtual void on_read(QByteArray &dt) = 0;
    virtual void on_write(QByteArray &dt) = 0;

private slots:
    void timerEvent(QTimerEvent * event){

        event = event;
        refresh();
    }

public slots:
    void answ_ack(int code = 0){

        QString ret = QString("OK");
        if(code) ret += QString(" %1").arg(code);
        ret += "\r\n";
        on_write(ret.toLatin1());
    }

    void answ_error(int code = 0, QString info = QString()){

        QString ret = QString("ERROR");
        if(code) ret += QString(" %1").arg(code);
        if(info.isEmpty() == false) ret += QString(", \"%2\"").arg(info);
        ret += "\r\n";
        on_write(ret.toLatin1());
    }


public:
    virtual void callback(unsigned ord, QString par){

        qDebug() << "ord(" << QString::number(ord) << ")" << par;
    }

    int refresh(){

        QByteArray dt;
        on_read(dt);

        int ret = -2; //== cekame na konec radky

        if(dt.isEmpty() == false)
            if((ret = feed(dt.constData(), dt.length())) >= 0){

                callback(ret, QString(get_parameters().c_str()));
                emit order(ret, QString(get_parameters().c_str()));
                return ret;
            }

        return ret;
    }

    void query_command(QString &cmd, int timeout){

        on_write(cmd.toLatin1());
        while(timeout > 0){

            if(refresh() >= 0)
                break;

            QMutex localMutex;
            localMutex.lock();
            QWaitCondition sleepSimulator;
            sleepSimulator.wait(&localMutex, 10);
            localMutex.unlock();

            timeout -= 10;
        }
    }


signals:
    void order(unsigned ord, QString par);

public:
    i_vi_comm_base(const char *orders[], QObject *parent = NULL) :
        QObject(parent),
        t_vi_comm_parser(orders)
    {
        sta = COMMSTA_UNKNOWN;

        if(VI_COMM_REFRESH_RT)
            this->startTimer(VI_COMM_REFRESH_RT);
    }

    virtual ~i_vi_comm_base(){

    }
};

#endif // I_COMMUNICATION_BASE

