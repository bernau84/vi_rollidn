#include <QCoreApplication>
#include "cmd_line/t_vi_comm_std_terminal.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList supported;
    supported << "START" << "STOP";

    t_vi_comm_std_terminal term(NULL, supported);
    term.refresh();

    return a.exec();
}

