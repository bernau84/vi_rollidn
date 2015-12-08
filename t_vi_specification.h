#ifndef T_VI_SPECIFICATION
#define T_VI_SPECIFICATION

#include "cinterface\tcp_uni\t_comm_tcp_uni.h"

//vyznam bitu definuje dokument "datagram-pl-pc-cpecification"
//https://docs.google.com/document/d/1mEbkwBnB_EEMSWxB8eQIjCiENeP74lsUJXaSv8vnYG8/edit

//const char *s_vi_plc_pc_ords[] = {
//    "MEAS", "ABORT", "OK", "RESULT", NULL
//};

//puvodne to byly ciala ale jakubneu to pochopil jako masku - budiz tedy
enum e_vi_plc_pc_ords {
    VI_PLC_PC_TRIGGER = (1 << 0),
    VI_PLC_PC_ABORT = (1 << 1),
    VI_PLC_PC_RESULT = (1 << 2),
    VI_PLC_PC_TRIGGER_ACK = (1 << 3),
    VI_PLC_PC_RESULT_ACK = (1 << 4),
    VI_PLC_PC_ERROR = (1 << 5), //see e_vi_plc_pc_errors
    VI_PLC_PC_READY = (1 << 6)
};


enum e_vi_plc_pc_errors {
    VI_ERR_OK = (1 << 0),
    VI_ERR_COMM_SYNTAX = (1 << 1),
    VI_ERR_COMM_TIMEOUT = (1 << 2),      //na prijem kompletniho datagramu mysleno
    VI_ERR_PLC1 = (1 << 3),
    VI_ERR_PLC2 = (1 << 4),
    VI_ERR_PLC3 = (1 << 5),
    VI_ERR_CAM_INIT = (1 << 10),
    VI_ERR_CAM_ACQ = (1 << 11),
    VI_ERR_CAM_OFFLINE = (1 << 12),
    VI_ERR_MEAS1 = (1 << 20),
    VI_ERR_MEAS2 = (1 << 21),
    VI_ERR_MEAS3 = (1 << 22),
};

#pragma pack(push,1)
struct t_comm_binary_rollidn {

    uint8_t ord;
    uint32_t flags;
    int width;
    int height;
};
#pragma pack(pop)


//verze tcp parseru a interface specificka pro ucely projektu roll-idn
//tvarime se jako tcp server a protokol je binarni
class t_comm_tcp_rollidn : public t_comm_tcp {

private:
    te_comm_parser_binary<t_comm_binary_rollidn> parser;

public:
    t_comm_tcp_rollidn(uint16_t port, QObject *parent = NULL):
        parser(),
        t_comm_tcp(port, &parser, parent)
    {
    }

    virtual ~t_comm_tcp_rollidn(){

    }

};

#endif // T_VI_SPECIFICATION

