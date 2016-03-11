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
#include "processing/t_vi_proc_roi_colortransf.h"
#include "processing/t_vi_proc_threshold_cont.h"
#include "processing/t_vi_proc_roll_approx.h"
#include "processing/t_vi_proc_sub_background.h"

#include "t_vi_setup.h"
#include "t_vi_specification.h"
#include "t_roll_idn_record_storage.h"

#define ERR_MEAS_MINAREA_TH 100
#define ERR_MEAS_MAXAREA_TH 1e+6

//global definition of txt remote orders
extern const char *ords[];

class t_roll_idn_collection : public QObject {

    Q_OBJECT

private:

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

    double mm_diameter;  //last final vaules in mm
    double mm_length;
    double area_min;    //minimalni plocha role v pixelech, vse pod je chyba
    double area_max;    //max plocha role v pixelech, vse nad je chyba
    double ref_luminance;   //referencni jas odecteny po ustaleni atoexpozice

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

    //little gin endian conv - for plc comm conventions
    uint32_t __to_rev_endian(uint32_t word){

        return ((word >> 24) & 0xFF) << 0 |
               ((word >> 16) & 0xFF) << 8 |
               ((word >> 8) & 0xFF) << 16 |
               ((word >> 0) & 0xFF) << 24;
    }

    //comapare errors in aproximation and mid line measurement, choose better
    //recalc to mm using focal length or calibration constant
    void __eval_measurement_res(){

        mm_diameter = 0;  //signal invalid measurement
        mm_length = 0;

        if((th.maxContRect.size.height * th.maxContRect.size.width) < area_min){

            error_mask |= VI_ERR_MEAS1;  //nepovedlo se zamerit polohu role ve scene
            log += QString("meas-error: 1(no ROI with roll)\r\n");
            return;
        }

        int raw_diameter = ms.midprof.diameter;  //berem stredni caru jako vychozi metodu
        int raw_length = ms.midprof.length;

        int overal_length_err_elipse = ms.eliptic.left_err + ms.eliptic.right_err;
        int overal_length_err_midline = ms.midprof.left_err + ms.midprof.right_err;

        if((ms.midprof.diameter * ms.midprof.length) < area_min){

            log += QString("meas-error: 2(midline unmeasured)\r\n");
            overal_length_err_midline = 1e+6; //bypass - tudle metodu nebrat
            error_mask |= VI_ERR_MEAS2;
        }

        if((ms.eliptic.diameter * ms.eliptic.length) < area_min){

            log += QString("meas-error: 2(eliptic unmeasured)\r\n");
            overal_length_err_elipse = 1e+6; //bypass - tudle metodu nebrat
            error_mask |= VI_ERR_MEAS3;
        }

        if((error_mask & VI_ERR_MEAS3) && (error_mask & VI_ERR_MEAS2)){

            log += QString("meas-error: 2+3(unmeasured)\r\n");
            return; //nepodarilo se odmerit ani jednou metodou - koncime
        }

        if(overal_length_err_elipse > ms.eliptic.diameter/5){

            log += QString("meas-error: 4(elipse err high)\r\n");
            error_mask |= VI_ERR_MEAS4;  //warning high diviation
        }

        if(overal_length_err_midline > ms.midprof.diameter/5){

            log += QString("meas-error: 5(midline err high)\r\n");
            error_mask |= VI_ERR_MEAS5;  //warning high diviation
        }

        if(overal_length_err_elipse < overal_length_err_midline){

            log += QString("meas-error: 6(unmeasured)\r\n");
            error_mask |= VI_ERR_MEAS6;      //fallback to elipse method indication

            raw_diameter = ms.eliptic.diameter;  //elipsy vychazeji lepe - berem je
            raw_length = ms.eliptic.length + ms.eliptic_left_radius + ms.eliptic_right_radius;
        }

        if(overal_length_err_elipse < overal_length_err_midline){

            raw_diameter = ms.eliptic.diameter;  //elipsy vychazeji lepe - berem je
            raw_length = ms.eliptic.length + ms.eliptic_left_radius + ms.eliptic_right_radius;
        }

        double ratio_l = par["calibr-x"].get().toDouble();
        double ratio_d = par["calibr-y"].get().toDouble();

        double f_l = par["focal-x"].get().toDouble();
        double f_d = par["focal-y"].get().toDouble();
        QList<QVariant> z_param = par["geometry-params"].get().toArray().toVariantList();

        if(f_l && f_d && z_param.size()){  //mame identifikovano ukameru a mame z ceho vypocitat vzdalenost sceny?

            double conv_d = z_param[0].toDouble() / f_d;
            double conv_l = z_param[0].toDouble() / f_l;

            mm_diameter = conv_d * raw_diameter;
            mm_length = conv_l * raw_length;

            if(z_param.size() > 1)
                if(z_param[1].isValid()){ //mame zpresnujici udaje o scene

                    /*! \todo - z uhlu pasu a ze zmerenho prumeru upravime conv_l */
                }

        } else if(ratio_l && ratio_d) {  //mame alespon hruby calibracni prepocet pix -> mm?

            mm_diameter = ratio_d * raw_diameter;
            mm_length = ratio_l * raw_length;
        } else {

            mm_diameter = raw_diameter;
            mm_length = raw_length;

            error_mask |= VI_ERR_MEAS7; //nemame jak udelat prepocet
            log += QString("meas-error: 7(uncalibred)\r\n");
            return;
        }

        log += QString("meas-x: w=%1[mm],pix=%2\r\n")
                .arg(mm_length)
                .arg(raw_length);

        log += QString("meas-y: h=%1[mm],pix=%2\r\n")
                .arg(mm_diameter)
                .arg(raw_diameter);
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

        ord_st.flags =  __to_rev_endian(ord_st.flags);
        ord_st.width = __to_rev_endian(ord_st.width);
        ord_st.height = __to_rev_endian(ord_st.height);

        log.clear();
        log += QString("<--rx: ord(%1),flags(0x%2),width(%3),height(%4)\r\n")
                .arg(unsigned(ord_st.ord))
                .arg(unsigned(ord_st.flags), 2, 16, QChar('0'))
                .arg(ord_st.width / 10.0)
                .arg(ord_st.height / 10.0);

        switch(ord){

            case VI_PLC_PC_TRIGGER: //meas
            {
                log += QString("<--rx: TRIGGER\r\n");

                store.increment();

                error_mask = VI_ERR_OK;

                //potvrdime prijem
                t_comm_binary_rollidn reply_st1 = {(uint16_t)VI_PLC_PC_TRIGGER_ACK, __to_rev_endian(error_mask), 0, 0};
                QByteArray reply_by1((const char *)&reply_st1, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by1); //tx ack to plc

                int res = on_trigger();
                if(!res) res = on_trigger(); //opakujem 2x pokud se mereni nepovede

                __eval_measurement_res();  //prepocet na mm

                uint32_t s_dia = mm_diameter * 10;
                uint32_t s_len = mm_length * 10;

                //odeslani vysledku
                t_comm_binary_rollidn reply_st2 = {(uint16_t)VI_PLC_PC_RESULT,
                                                   __to_rev_endian(error_mask),
                                                   __to_rev_endian(s_len),
                                                   __to_rev_endian(s_dia)};
                QByteArray reply_by2((const char *)&reply_st2, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by2);  //tx results to plc

                log += QString("-->tx: RESULT\r\n");
                log += QString("-->tx: ord(%1),flags(0x%2),len(%3),dia(%4)\r\n")
                        .arg(unsigned(reply_st2.ord))
                        .arg(unsigned(reply_st2.flags), 2, 16, QChar('0'))
                        .arg(mm_diameter)
                        .arg(mm_length);
            }
            break;
            case VI_PLC_PC_ABORT:
                log += QString("<--rx: ABORT\r\n");
                on_abort(); //nastavi preruseni a ceka na jeho vyhodnoceni
                //a prekontrolujem jak na tom sme
            break;
            case VI_PLC_PC_READY:
            {
                log += QString("<--rx: READY\r\n");
                if(on_ready()){
                    //potvrdime prijem
                    log += QString("-->tx: READY\r\n");
                    t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_READY, 0/*error_mask*/, 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                } else {
                    //nejsme operabilni
                    log += QString("-->tx: ERROR\r\n");
                    t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_ERROR,
                                                       __to_rev_endian(error_mask), 0, 0};
                    QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                    iface.on_write(reply_by);
                }
            }
            break;
            case VI_PLC_PC_BACKGROUND:
            {

                error_mask = VI_ERR_OK;

                if(cam_device.sta != i_vi_camera_base::CAMSTA_PREPARED)
                    error_mask |= VI_ERR_CAM_NOTFOUND;
                else if(cam_device.exposure(-100)){ //100us tolerance to settling exposure

                    on_trigger(true); //true == background mode
                    if(error_mask == VI_ERR_OK){

                    }
                }
                //odvysilame vysledek
                log += QString("-->tx: BACKGROUND_ACK\r\n");
                t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_BACKGROUND_ACK,
                                                  __to_rev_endian(error_mask), 0, 0};
                QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by);
            }
            break;
            case VI_PLC_PC_CALIBRATE:
            {
                log += QString("<--rx: CALIBRATE\r\n");
                store.increment();

                error_mask = VI_ERR_OK;

                on_calibration();

                if((th.maxContRect.size.height * th.maxContRect.size.width) < area_min)
                    error_mask |= VI_ERR_MEAS1;

                if((ms.eliptic.diameter * ms.eliptic.length) < area_min)
                    error_mask |= VI_ERR_MEAS2;

                if(error_mask == VI_ERR_OK){

                    //count new scale factor + save in config
                    double c1d = (ord_st.width / 10.0) / ms.eliptic.length;
                    t_setup_entry c1; par.ask("calibr-x", &c1);
                    c1.set(c1d);
                    par.replace("calibr-x", c1);

                    log += QString("cal-x: ref=%1[mm],pix=%2,ratio=%3\r\n")
                            .arg(ord_st.width / 10.0)
                            .arg(ms.eliptic.length)
                            .arg(c1d);

                    //count new scale factor + save in config
                    double c2d = (ord_st.height / 10.0) / ms.eliptic.diameter;
                    t_setup_entry c2; par.ask("calibr-y", &c2);
                    c2.set(c2d);
                    par.replace("calibr-y", c2);

                    log += QString("cal-y: ref=%1[mm],pix=%2,ratio=%3\r\n")
                            .arg(ord_st.height / 10.0)
                            .arg(ms.eliptic.diameter)
                            .arg(c2d);

                    __to_file();
                }

                //potvrdime vysledek - pokud se nepovedlo vratime nejaky error bit + nesmyslne hodnoty mereni width & height
                //jinak hodnoty v raw == v pixelech
                t_comm_binary_rollidn reply_st = {(uint16_t)VI_PLC_PC_CALIBRATE_ACK,
                                                  __to_rev_endian(error_mask),
                                                  __to_rev_endian(ms.eliptic.length),
                                                  __to_rev_endian(ms.eliptic.diameter)};
                QByteArray reply_by((const char *)&reply_st, sizeof(t_comm_binary_rollidn));
                iface.on_write(reply_by);

                log += QString("-->tx: CALIBRATE_ACK\r\n");
                log += QString("-->tx: ord(%1),flags(0x%2),len(%3),dia(%4)\r\n")
                        .arg(unsigned(reply_st.ord))
                        .arg(unsigned(reply_st.flags), 2, 16, QChar('0'))
                        .arg(ms.eliptic.length)
                        .arg(ms.eliptic.diameter);
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

            if(cam_device.sta == i_vi_camera_base::CAMSTA_PREPARED){

                pisize = cam_device.snap(img, 4000 * 3000 * 4, &info);
            } else {

                error_mask |= VI_ERR_CAM_NOTFOUND;
                pisize = cam_simul.snap(img, 4000 * 3000 * 4, &info);
            }

            if((rep >= 5) || abort){

                error_mask |= VI_ERR_CAM_TIMEOUT;
                switch(pisize){

                    case 0:     error_mask |= VI_ERR_CAM_SNAPERR;   break;
                    case -101:  error_mask |= VI_ERR_CAM_BADPICT;   break;
                    case -102:  error_mask |= VI_ERR_CAM_EXCEPTION; break;
                }

                log += QString("cam-error: timeout / abort");
                abort = false;
                return 0;
            }

            QEventLoop loop;  //process pottential abort
            loop.processEvents();
        }

        if(info.w * info.h <= 0){

            error_mask |= VI_ERR_CAM_BADPICT;
            log += QString("cam-error: bad picture");
            return 0;
        }

        snapshot = QImage(img, info.w, info.h, (QImage::Format)info.format);
        store.insert(snapshot);

        //process measurement or save new background
        cv::Mat src(info.h, info.w, CV_8UC4, img);
        int order = (background) ? t_vi_proc_sub_backgr::SUBBCK_REFRESH : t_vi_proc_sub_backgr::SUBBCK_SUBSTRACT;
        bc.proc(order, &src);

        delete[] img;
        return 1;
    }

    int on_abort(){

        abort = true;

        /*! \todo - qloop a cekame na vyhodnoceni (smazani abortu) */
        return 1;
    }

    int on_ready(){

        //zafixujeme nastaveni expozice a ulozime si referencni hodnotu jasu
        if(ref_luminance < 0){

            if(cam_device.sta != i_vi_camera_base::CAMSTA_PREPARED)
                error_mask |= VI_ERR_CAM_NOTFOUND;
            else if(cam_device.exposure(-100)){ //100us tolerance to settling exposure

                on_trigger(true); //true == background mode
                if(error_mask == VI_ERR_OK){

                }
            }
        }

        /*! \todo - vyhodnotit stav - mame zkalibravano nebo ne; inicializace a nastaveni chyb */
        return 1;
    }

    int on_calibration(){

        return on_trigger();
    }

public:

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

        if(0 >= (area_min = par["contour_minimal"].get().toDouble()))
            area_min = ERR_MEAS_MINAREA_TH;

        if(0 >= (area_max = par["contour_maximal"].get().toDouble()))
            area_max = ERR_MEAS_MAXAREA_TH;

        ref_luminance = -1;  //indikuje ze nebylo dosud provedeno
    }

    ~t_roll_idn_collection(){

    }
};

#endif // T_ROLL_IDN_COLLECTION

