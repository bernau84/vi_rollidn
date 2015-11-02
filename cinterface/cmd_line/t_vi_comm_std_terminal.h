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

        if(std::cin.rdbuf()->in_avail() == 0)
            return;

        char c = std::cin.get();  //v terminalu qt creatoru + win to zrovna nefunguje, ani cin.readsome(&c, 1)
        if(c) dt.append(c);
    }

    virtual void on_write(QByteArray &dt){

        std::cout << dt.toStdString();
    }

    t_vi_comm_std_terminal(const char *orders[] = NULL, QObject *parent = NULL):
        i_vi_comm_base(orders, parent)
    {

    }

    ~t_vi_comm_std_terminal(){

    }
};

#endif // T_VI_COMM_STD_TERMINAL

