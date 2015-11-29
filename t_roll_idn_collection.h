#ifndef T_ROLL_IDN_COLLECTION
#define T_ROLL_IDN_COLLECTION

#include <QObject>
#include <QLabel>
#include <QEventLoop>

#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cinterface/cmd_line/t_vi_comm_std_terminal.h"
#include "cinterface/tcp_client/t_vi_comm_tcp_uni.h"
#include "cameras/basler/t_vi_camera_basler_usb.h"
#include "cameras/offline/t_vi_camera_offline_file.h"
#include "processing/t_vi_proc_roi_grayscale.h"
#include "processing/t_vi_proc_threshold_cont.h"
#include "processing/t_vi_proc_roll_approx.h"

#include "t_vi_setup.h"
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

    int on_order(unsigned ord, QString par){

        par = par; //unused
        switch(ord){

            case 0: //meas
            {
                int res = on_trigger();
               //opakujem 2x pokud se mereni nepovede
                if(!res) res = on_trigger();
                return res;
            }
            break;
            case 1:
                return on_abort();
            break;
        }

        return 0;
    }

    int on_trigger(){

        uint8_t *img = (uint8_t *) new uint8_t[4 * 4000 * 3000];
        i_vi_camera_base::t_campic_info info;

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

        /// Load source image and convert it to gray
        cv::Mat src(info.h, info.w, CV_8UC4, img);

        ct.proc(0, &src);

        //debug
        const char *replys = "ZMERENO!";
        QByteArray replyb(replys);
        tcp_server.on_write(replyb);

        store.add(QString(replys), snapshot);

        delete[] img;

        return ((ms.height > 0) && (ms.width > 0)) ? 1 : 0;
    }

    int on_abort(){

        abort = true;
        return 1;
    }

public:
    t_vi_comm_tcp_uni tcp_server;
    t_vi_camera_basler_usb cam_device;
    t_vi_camera_offline_file cam_simul;

    t_collection par;
    bool abort;

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_roll_approx ms;

    t_roll_idn_record_storage store;

    int initialize(){

        cam_device.init();
        cam_simul.init();

#ifndef QT_DEBUG

        //cekani na pripojeni plc
        QEventLoop loop;
        while(tcp_server.health() != COMMSTA_PREPARED)
            loop.processEvents();
#endif //

        return 1;
    }

    t_roll_idn_collection(QString &js_config, QObject *parent = NULL):
        QObject(parent),
        par(__set_from_file(js_config)),
        abort(false),
        tcp_server(9100, NULL /*ords*/),  //ted to parsovat nebude - stejne sme to tak nechteli
        ct(js_config),
        th(js_config),
        ms(js_config),
        store(QDir::currentPath() + "/storage")
    {
        //zretezeni analyz
        QObject::connect(&ct, SIGNAL(next(int, void *)), &th, SLOT(proc(int, void *)));
        QObject::connect(&th, SIGNAL(next(int, void *)), &ms, SLOT(proc(int, void *)));

        //z vnejsu vyvolana akce
        QObject::connect(&tcp_server, SIGNAL(order(unsigned, QString)), this, SLOT(on_order(unsigned, QString)));
    }

    ~t_roll_idn_collection(){

    }
};

#endif // T_ROLL_IDN_COLLECTION

