#ifndef T_VII_COMMAND_PARSER
#define T_VII_COMMAND_PARSER

#include <stdint.h>
#include <string.h>

class t_vi_comm_parser {

private:
    std::vector<const char *> orders;
    std::string line;
    std::string last_par;
    std::string last_ord;

public:
    //nova data pro parser - hleda povely ve streamu dat
    //a generuje signal pokud najdem
    //povel musi byt ukoncem <CR> nebo <LF> nebo oboji
    //-1 povel neexistuje
    //-2 cekame na konec radky
    int feed(const char *p, unsigned len){

        while(*p && len){

            if((*p == '\r') || (*p == '\n')){

                for(unsigned i=0; i<orders.size(); i++){

                    int sz = strlen(orders[i]);
                    if(0 == memcmp(orders[i], line.c_str(), sz)){

                        last_ord = std::string(orders[i]);
                        last_par = line.substr(sz, line.length()  - sz);

                        line.erase();
                        return i;
                    }
                }

                line.erase();
                return -1;
            } else {

                line.push_back(*p);
            }

            p += 1;
            len -= 1;
        }

        return -2;
    }

    //vraci adekvatni string posledniho povelu
    std::string get_order(){

        return last_ord;
    }

    //vraci parametry posledniho povelu
    std::string get_parameters(){

        return last_par;
    }

    //vraci kod pro registrovany povel
    unsigned reg_command(const char *ord){

        orders.push_back(ord);
        return orders.size();
    }

    t_vi_comm_parser(const char *set[])
    {
        if(set)
            for(int i=0; (set[i]) && (*set[i]); i++)
                reg_command(set[i]);
    }

    t_vi_comm_parser()
    {
    }

    ~t_vi_comm_parser()
    {
    }
};

#endif // T_VII_COMMAND_PARSER

