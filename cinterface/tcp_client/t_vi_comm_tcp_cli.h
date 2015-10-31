#ifndef T_VI_COMM_TCP_CLI
#define T_VI_COMM_TCP_CLI

#include "../i_comm_base.h"
#include "../../t_vi_setup.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QUrl>

class t_vi_comm_tcp_cli : public i_vi_comm_base {

    Q_OBJECT

protected:
    t_collection par;
    QTcpSocket *tcp;
    QTcpServer ser;

private:
    void connect_if_unconnected(){

        if(tcp != NULL)
            return;

        QUrl url(par["Host"].get().toString());
        if(false == url.isValid()){

            uint16_t port = par["Port"].get().toInt();
            if(false == ser.isListening())
                ser.listen(QHostAddress::Any, port);

            qDebug() << "Listen on " << port << "port";
        } else {

            tcp = (QTcpSocket *) new QTcpSocket(this);  //uvolnime s timto objektem
            tcp->connectToHost(url.host(), url.port());
            if(tcp->waitForConnected(5000)){

                qDebug << "Connected!";
            } else {

                qDebug << "Connect timeout!";
                tcp = NULL;
            }
        }
    }

private slots:

    void accept(void){

        tcp = QTcpServer::nextPendingConnection();
        qDebug << "Connected!";
    }

public:
    virtual void on_read(QByteArray &dt){

        connect_if_unconnected();

        if(tcp)
            if(tcp->isReadable())
                dt = tcp->readAll();
    }

    virtual void on_write(QByteArray &dt){

        connect_if_unconnected();

        if(tcp)
            if(tcp->isWritable())
                tcp->write(dt);
    }

    t_vi_comm_tcp_cli(QObject *parent = NULL, QStringList orders = QStringList()):
        i_vi_comm_base(parent, orders),
        tcp(NULL),
        ser()
    {
        connect(&ser, SIGNAL(newConnection()), this, SLOT(accept()));
    }

    ~t_vi_comm_tcp_cli(){

    }
};

#endif // T_VI_COMM_TCP_CLI

