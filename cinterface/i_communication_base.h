#ifndef I_COMMUNICATION_BASE
#define I_COMMUNICATION_BASE

#include <stdio.h>
#include <stdint.h>

#include <QObject>
#include <QString>

#include "t_vi_command_parser.h"

using namespace std;

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

    void on_ack(int code = 0){

        QString ret = QString("OK");
        if(code) ret += QString(" %1").arg(code);
        ret += "\r\n";
        on_write(ret);
    }

    void on_error(int code = 0, QString info = QString()){

        QString ret = QString("ERROR");
        if(code) ret += QString(" %1").arg(code);
        if(info.isEmpty() == false) ret += QString(", \"%2\"").arg(info);
        ret += "\r\n";
        on_write(ret);
    }

    void on_answer_or_command(QByteArray &cmd){

        on_write(cmd);
    }

    int refresh(){

        QByteArray dt;
        on_read(dt);

        int ret = -1;

        if(dt.isEmpty() == false)
            if((ret = feed(dt.constData(), dt.length())) >= 0){

                callback(ret, QString(parser.get_parameteres()));
                return ret;
            }

        return -1;
    }

    virtual callback(unsigned ord, QString par){

        ord = ord;
        par = par;
    }

signals:
    void order(unsigned ord, QString par);

public:
    i_vi_comm_base(QObject *parent = NULL, QStringList orders = QStringList()) :
        QObject(parent),
        t_vi_comm_parser()
    {
        for(int i=0; i<orders.size(); i++)
            this->reg_command(orders[i].toStdString());

        sta = COMMSTA_UNKNOWN;
    }

    virtual ~i_vi_comm_base(){

    }
}

#endif // I_COMMUNICATION_BASE

