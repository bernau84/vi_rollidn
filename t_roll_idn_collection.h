#ifndef T_ROLL_IDN_COLLECTION
#define T_ROLL_IDN_COLLECTION

#include <QObject>
#include <QLabel>
#include <QEventLoop>

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

#include "t_vi_setup.h"
#include "t_vi_specification.h"
#include "t_roll_idn_record_storage.h"

//global definition of txt remote orders
extern const char *ords[];

class t_roll_idn_collection : public QObject {

    Q_OBJECT

private:
    //constructor helpers
    QJsonObject __set_from_file(const QString &path){

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

public slots:

    int on_done(int res, void *img){

        res = res;
        img = img;
        return 0;
    }

    //slot co se zavola s prijmem povelu od plc
    int on_order(unsigned ord, QByteArray par){

        par = par; //unused
        switch(ord){

            case VI_PLC_PC_TRIGGER: //meas
            {
                //potvrdime prijem
                t_comm_binary_rollidn reply_st1 = {(uint8_t)VI_PLC_PC_TRIGGER_ACK, error_mask, 0, 0};
                QByteArray reply_by1((const char *)&reply_st1, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by1);

                int res = on_trigger();
                if(!res) res = on_trigger(); //opakujem 2x pokud se mereni nepovede

                t_comm_binary_rollidn reply_st2 = {(uint8_t)VI_PLC_PC_RESULT, error_mask, ms.width, ms.height};
                QByteArray reply_by2((const char *)&reply_st2, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by2);

                return res;
            }
            break;
            case VI_PLC_PC_ABORT:
                on_abort(); //nastavi preruseni a ceka na jeho vyuhodnoceni
                //a prekontrolujem jak na tom sme
            case VI_PLC_PC_READY:
            {
                if(on_ready()){
                    //potvrdime prijem
                    t_comm_binary_rollidn reply_st = {(uint8_t)VI_PLC_PC_CALIBRATE_ACK, error_mask, 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                } else {
                    //nejsme operabilni
                    t_comm_binary_rollidn reply_st = {(uint8_t)VI_PLC_PC_ERROR, error_mask, 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                }
            }
            break;
            case VI_PLC_PC_CALIBRATE:
            {
                on_calibration();

                //potvrdime vysledek - pokud se nepovedlo vratime nejaky error bit + nesmyslne hodnoty mereni width & height
                t_comm_binary_rollidn reply_st = {(uint8_t)VI_PLC_PC_CALIBRATE_ACK, error_mask, ms.width, ms.height};
                QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by);
            }
            break;
        }

        return 0;
    }

    //odpaleni snimani analyza a odreportovani vysledku
    //volame zatim rucne
    int on_trigger(){

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

        QImage snapshot(img, info.w, info.h, (QImage::Format)info.format);

        QLabel vizual;
        vizual.setPixmap(QPixmap::fromImage(snapshot));
        vizual.show();

        cv::Mat src(info.h, info.w, CV_8UC4, img);
        ct.proc(0, &src);

        delete[] img;

        //vyhodnoceni vysledku
        int res = 1;
        /*! \todo - kouknem na zmerene rozmery, konturu */

        return res;
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

        /*! \todo - volame triger a prepocet kalibracnich konstant
        zapisem do konfigurace
        */
        return 1;
    }

public:
    t_collection par;
    bool abort;
    uint32_t error_mask;  //suma flagu e_vi_plc_pc_errors

    t_vi_camera_basler_usb cam_device;
    t_vi_camera_offline_file cam_simul;

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_roll_approx ms;

    t_comm_tcp_rollidn iface;

    t_roll_idn_record_storage store;

    int initialize(){

        cam_device.init();
        cam_simul.init();

//#ifndef QT_DEBUG
//        //cekani na pripojeni plc
//        QEventLoop loop;
//        while(iface.health() != COMMSTA_PREPARED)
//            loop.processEvents();
//#endif

        return 1;
    }

    t_roll_idn_collection(QString &js_config, QObject *parent = NULL):
        QObject(parent),
        par(__set_from_file(js_config)),
        abort(false),
        ct(js_config),
        th(js_config),
        ms(js_config),
        iface(par["tcp-server-port"].get().toInt(), this),
        store(QDir::currentPath() + "/storage")
    {
        //zretezeni analyz
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

