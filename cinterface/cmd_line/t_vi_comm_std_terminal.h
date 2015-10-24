#ifndef T_VI_COMM_STD_TERMINAL
#define T_VI_COMM_STD_TERMINAL

#include "../i_comm_base.h"
#include "../../t_vi_setup.h"

#include <iostream>

class t_vi_comm_std_terminal : public i_vi_comm_base {

protected:
    t_collection par;

public:
    virtual void on_read(QByteArray &dt){

        char c;
        if(std::cin.readsome(&c, 1))
           dt.append(c);
    }

    virtual void on_write(QByteArray &dt){

        std::cout << dt.toStdString();
    }

    t_vi_comm_std_terminal(QObject *parent = NULL, QStringList orders = QStringList()):
        i_vi_comm_base(parent, orders)
    {

    }

    ~t_vi_comm_std_terminal(){

    }
};

#endif // T_VI_COMM_STD_TERMINAL

