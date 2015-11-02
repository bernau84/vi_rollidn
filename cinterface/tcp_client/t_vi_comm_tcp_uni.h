#ifndef T_VI_COMM_TCP_CLI
#define T_VI_COMM_TCP_CLI

#include "../i_comm_base.h"
#include "../../t_vi_setup.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QUrl>

class t_vi_comm_tcp_uni : public i_vi_comm_base {

    Q_OBJECT

protected:
    QTcpSocket *tcp;
    QTcpServer ser;

    QUrl m_url;
    uint16_t m_port;

private:
    void connect_if_unconnected(){

        if(tcp != NULL)
            return;

        if(false == m_url.isValid()){

            uint16_t port = m_port;
            if(port == 0) port = 23232;  //"random"

            if(false == ser.isListening()){

                ser.listen(QHostAddress::Any, port);
                qDebug() << "Listen on " << port << "port";
            }
        } else {

            tcp = (QTcpSocket *) new QTcpSocket(this);  //uvolnime s timto objektem
            QString host = m_url.host();
            int port = m_url.port();
            tcp->connectToHost(host, port);
            if(tcp->waitForConnected(50000)){

                qDebug() << "Connected!";
            } else {

                qDebug() << "Connect timeout!";
                tcp = NULL;
            }
        }
    }

private slots:

    void accept(void){

        tcp = ser.nextPendingConnection();
        qDebug() << "Connected!";
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

    /*
     * server mode
     */
    t_vi_comm_tcp_uni(uint16_t port, const char *orders[] = NULL, QObject *parent = NULL):
        i_vi_comm_base(orders, parent),
        tcp(NULL),
        ser(),
        m_port(port)
    {
        connect(&ser, SIGNAL(newConnection()), this, SLOT(accept()));
    }

    /*
     * client mode
     */
    t_vi_comm_tcp_uni(QUrl &url, const char *orders[] = NULL, QObject *parent = NULL):
        i_vi_comm_base(orders, parent),
        tcp(NULL),
        ser(),
        m_url(url)
    {
        //connect(&ser, SIGNAL(newConnection()), this, SLOT(accept()));  //not need
    }

    ~t_vi_comm_tcp_uni(){

    }
};

#endif // T_VI_COMM_TCP_CLI

