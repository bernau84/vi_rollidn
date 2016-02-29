#ifndef T_ROLL_IDN_COLLECTION
#define T_ROLL_IDN_COLLECTION

#include <QObject>
#include <QEventLoop>
#include <QtEndian>

#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cinterface/i_comm_generic.h"
#include "cameras/basler/t_vi_camera_basler_usb.h"
#include "cameras/offline/t_vi_camera_offline_file.h"
#include "processing/t_vi_proc_roi_grayscale.h"
#include "processing/t_vi_proc_threshold_cont.h"
#include "processing/t_vi_proc_roll_approx.h"
#include "processing/t_vi_proc_sub_background.h"

#include "t_vi_setup.h"
#include "t_vi_specification.h"
#include "t_roll_idn_record_storage.h"

#define ERR_MEAS_MINAREA_TH 100

//global definition of txt remote orders
extern const char *ords[];

class t_roll_idn_collection : public QObject {

    Q_OBJECT

private:
    //constructor helpers
    QJsonObject __from_file(){

        // default config
        QFile f_def(path);  //from resources
        if(f_def.open(QIODevice::ReadOnly | QIODevice::Text)){

            QByteArray f_data = f_def.read(64000);

            QJsonDocument js_doc = QJsonDocument::fromJson(f_data);
            if(!js_doc.isEmpty()){

                //qDebug() << js_doc.toJson();
                return js_doc.object();
            }
        }

        return QJsonObject();
    }

    //constructor helpers
    bool __to_file(){

        // default config
        QFile f_def(path);  //from resources
        if(f_def.open(QIODevice::WriteOnly | QIODevice::Text)){

            //if(!par.isEmpty()){

                QJsonDocument js_doc(par);
                f_def.write(js_doc.toJson());
                return true;
            //}
        }

        return false;
    }

    uint32_t __to_rev_endian(uint32_t word){

        return ((word >> 24) & 0xFF) << 0 |
               ((word >> 16) & 0xFF) << 8 |
               ((word >> 8) & 0xFF) << 16 |
               ((word >> 0) & 0xFF) << 24;
    }

public slots:

    int on_done(int res, void *img){

        res = res;
        img = img;
        return 0;
    }

    //slot co se zavola s prijmem povelu od plc
    int on_order(unsigned ord, QByteArray raw){

        t_comm_binary_rollidn ord_st;
        memcpy(&ord_st, raw.data(), sizeof(t_comm_binary_rollidn));

        ord_st.height = __to_rev_endian(ord_st.width);
        ord_st.width = __to_rev_endian(ord_st.height);

        log.clear();
        log += QString("rx: ord(%1),flags(0x%2),width(%3),height(%4)\r\n")
                .arg(unsigned(ord_st.ord))
                .arg(unsigned(ord_st.flags), 2, 16, QChar('0'))
                .arg(ord_st.width / 10.0)
                .arg(ord_st.height / 10.0);

        switch(ord){

            case VI_PLC_PC_TRIGGER: //meas
            {
                log += QString("rx: TRIGGER\r\n");

                store.increment();

                //potvrdime prijem
                error_mask = VI_ERR_OK;

                t_comm_binary_rollidn reply_st1 = {(uint16_t)VI_PLC_PC_TRIGGER_ACK, error_mask, 0, 0};
                QByteArray reply_by1((const char *)&reply_st1, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by1);

                int res = on_trigger();
                if(!res) res = on_trigger(); //opakujem 2x pokud se mereni nepovede

                if((th.maxContRect.size.height * th.maxContRect.size.width) < ERR_MEAS_MINAREA_TH)
                    error_mask |= VI_ERR_MEAS1;

                if((ms.eliptic.diameter * ms.eliptic.length) < ERR_MEAS_MINAREA_TH)
                    error_mask |= VI_ERR_MEAS2;

                double ratio_l = par["calibr-L"].get().toDouble();
                double ratio_d = par["calibr-D"].get().toDouble();

                log += QString("meas-x: w=%1[mm],pix=%2,ratio=%3\r\n")
                        .arg(ms.eliptic.length * ratio_l)
                        .arg(ms.eliptic.length)
                        .arg(ratio_l);

                log += QString("meas-y: h=%1[mm],pix=%2,ratio=%3\r\n")
                        .arg(ms.eliptic.diameter * ratio_d)
                        .arg(ms.eliptic.diameter)
                        .arg(ratio_d);

                ms.eliptic.diameter *= ratio_d;
                ms.eliptic.length *= ratio_l;

                uint32_t s_dia = ms.eliptic.diameter * 10;
                uint32_t s_len = ms.eliptic.length * 10;

                t_comm_binary_rollidn reply_st2 = {(uint16_t)VI_PLC_PC_RESULT, error_mask,
                                                   __to_rev_endian(s_len),
                                                   __to_rev_endian(s_dia)};
                QByteArray reply_by2((const char *)&reply_st2, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by2);

                log += QString("tx: RESULT\r\n");
                log += QString("tx: ord(%1),flags(0x%2),len(%3),dia(%4)\r\n")
                        .arg(unsigned(reply_st2.ord))
                        .arg(unsigned(reply_st2.flags), 2, 16, QChar('0'))
                        .arg(reply_st2.width)
                        .arg(reply_st2.height);
            }
            break;
            case VI_PLC_PC_ABORT:
                log += QString("rx: ABORT\r\n");
                on_abort(); //nastavi preruseni a ceka na jeho vyhodnoceni
                //a prekontrolujem jak na tom sme
            break;
            case VI_PLC_PC_READY:
            {
                log += QString("rx: READY\r\n");
                if(on_ready()){
                    //potvrdime prijem
                    log += QString("tx: READY\r\n");
                    t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_READY, 0/*error_mask*/, 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                } else {
                    //nejsme operabilni
                    log += QString("tx: ERROR\r\n");
                    t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_ERROR, error_mask, 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                }
            }
            break;
            case VI_PLC_PC_BACKGROUND:
                //if(cam_device == basler) /*! \todo */
                if(cam_device.exposure(-100)){ //100us tolerance to settling exposure

                    on_trigger(true); //true == background mode
                    if(error_mask == VI_ERR_OK){


                    }
                }

            break;
            case VI_PLC_PC_CALIBRATE:
            {
                log += QString("rx: CALIBRATE\r\n");
                store.increment();

                error_mask = VI_ERR_OK;

                on_calibration();

                if((th.maxContRect.size.height * th.maxContRect.size.width) < ERR_MEAS_MINAREA_TH)
                    error_mask |= VI_ERR_MEAS1;

                if((ms.eliptic.diameter * ms.eliptic.length) < ERR_MEAS_MINAREA_TH)
                    error_mask |= VI_ERR_MEAS2;

                if(error_mask == VI_ERR_OK){

                    double c1d = (ord_st.width / 10.0) / ms.eliptic.length;
                    t_setup_entry c1; par.ask("calibr-L", &c1);
                    c1.set(c1d);
                    par.replace("calibr-L", c1);

                    log += QString("cal-x: ref=%1[mm],pix=%2,ratio=%3\r\n")
                            .arg(ord_st.width)
                            .arg(ms.eliptic.length)
                            .arg(ord_st.width / ms.eliptic.length);

                    double c2d = (ord_st.height / 10.0) / ms.eliptic.diameter;
                    t_setup_entry c2; par.ask("calibr-D", &c2);
                    c2.set(c2d);
                    par.replace("calibr-D", c2);

                    log += QString("cal-y: ref=%1[mm],pix=%2,ratio=%3\r\n")
                            .arg(ord_st.height)
                            .arg(ms.eliptic.diameter)
                            .arg(ord_st.height / ms.eliptic.diameter);

                    __to_file();
                }

                //potvrdime vysledek - pokud se nepovedlo vratime nejaky error bit + nesmyslne hodnoty mereni width & height
                t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_CALIBRATE_ACK, error_mask, ms.eliptic.length, ms.eliptic.diameter};
                QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by);

                log += QString("tx: CALIBRATE_ACK\r\n");
                log += QString("tx: ord(%1),flags(0x%2),len(%3),dia(%4)\r\n")
                        .arg(unsigned(reply_st.ord))
                        .arg(unsigned(reply_st.flags), 2, 16, QChar('0'))
                        .arg(reply_st.width)
                        .arg(reply_st.height);
            }
            break;
        }

        store.append(log);
        return 0;
    }

    //odpaleni snimani analyza a odreportovani vysledku
    //volame zatim rucne
    int on_trigger(bool background = false){

        uint8_t *img = (uint8_t *) new uint8_t[4 * 4000 * 3000];
        i_vi_camera_base::t_campic_info info;

        //acquisition
        int pisize = 0;
        for(int rep = 0; pisize <= 0; rep++){

            if(cam_device.sta == i_vi_camera_base::CAMSTA_PREPARED)
                pisize = cam_device.snap(img, 4000 * 3000 * 4, &info);
            else
                pisize = cam_simul.snap(img, 4000 * 3000 * 4, &info);

            if((rep >= 5) || abort){

                /*! \todo - vyhlasime chybu */
                abort = false;
                return 0;
            }

            QEventLoop loop;  //process pottential abort
            loop.processEvents();
        }

        snapshot = QImage(img, info.w, info.h, (QImage::Format)info.format);
        store.insert(snapshot);

        cv::Mat src(info.h, info.w, CV_8UC4, img);

        //process measurement or save new background
        bc.proc((background) ? 1 : 0, &src);

        delete[] img;
        return 1;
    }

    int on_abort(){

        abort = true;

        /*! \todo - qloop a cekame na vyhodnoceni (smazani abortu) */

        return 1;
    }

    int on_ready(){

        /*! \todo - vyhodnotit stav - mame zkalibravano nebo ne; inicializace a nastaveni chyb */
        return 1;
    }

    int on_calibration(){

        /*! \todo chessboard full calibration */
        //return on_trigger();
        return 0;
    }

public:
    QImage snapshot;
    QString path;
    QString log;

    t_collection par;
    bool abort;
    uint32_t error_mask;  //suma flagu e_vi_plc_pc_errors

    t_vi_camera_basler_usb cam_device;
    t_vi_camera_offline_file cam_simul;

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_roll_approx ms;
    t_vi_proc_sub_backgr bc;

    t_comm_tcp_rollidn iface;

    t_roll_idn_record_storage store;

    int initialize(){

        cam_device.init();
        cam_simul.init();
        return 1;
    }

    t_roll_idn_collection(QString &js_config, QObject *parent = NULL):
        QObject(parent),
        path(js_config),
        par(__from_file()),
        cam_device(path),
        cam_simul(path),
        abort(false),
        ct(path),
        th(path),
        ms(path),
        bc(path),
        iface(par["tcp-server-port"].get().toInt(), this),
        store(QDir::currentPath() + "/storage")
    {
        //zretezeni analyz
        QObject::connect(&bc, SIGNAL(next(int, void *)), &ct, SLOT(proc(int, void *)));
        QObject::connect(&ct, SIGNAL(next(int, void *)), &th, SLOT(proc(int, void *)));
        QObject::connect(&th, SIGNAL(next(int, void *)), &ms, SLOT(proc(int, void *)));

        /*! \todo - navazat vystupem ms na ulozeni vysledku analyzy (obrazek) */

        //z vnejsu vyvolana akce
        QObject::connect(&iface, SIGNAL(order(unsigned, QByteArray)), this, SLOT(on_order(unsigned, QByteArray)));
    }

    ~t_roll_idn_collection(){

    }
};

#endif // T_ROLL_IDN_COLLECTION

