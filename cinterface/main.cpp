#include <QCoreApplication>
#include "cmd_line/t_vi_comm_std_terminal.h"
#include "tcp_client/t_vi_comm_tcp_uni.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    const char *ords[] = {
        "MEAS", "ABORT", "OK", "RESULT", NULL
    };

    //t_vi_test_std_term term;
    t_vi_comm_tcp_uni rem_serv(55000, ords);
    t_vi_comm_tcp_uni loc_cli(QUrl("http://localhost:55000"), ords);

    rem_serv.query_command(QString("INIT\r\n"), 10);
    loc_cli.query_command(QString("RESULT\r\n"), 10);

    return a.exec();
}

