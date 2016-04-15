#ifndef I_CAMERA_BASE
#define I_CAMERA_BASE

#include <stdio.h>
#include <stdint.h>
#include "t_camera_attrib.h"

#include <QImage>
#include <QElapsedTimer>

using namespace std;

class i_vi_camera_base {

public:
    enum e_camsta {

        CAMSTA_UNKNOWN = 0,
        CAMSTA_PREPARED,
        CAMSTA_INPROC,
        CAMSTA_ERROR
    } sta;

    /*! tyka se metody exposition()
     */
    enum e_cam_value {

        CAMVAL_UNDEF = 0,
        CAMVAL_ABS = 1,
        CAMVAL_AUTO_TOLERANCE,
        CAMVAL_AUTO_TIMEOUT
    };

    struct t_campic_info {

        unsigned h;
        unsigned w;
        unsigned format;
    };


protected:
    QElapsedTimer etimer;
    qint64 conv_elapsed;
    qint64 snap_elapsed;
    t_vi_camera_attrib par;     //camera attributes

    /*! \brief interface for taking pictures */
    virtual int isnap(void *img, unsigned free, t_campic_info *info = NULL) = 0;

    /*! \brief setup collection io, read for empty/default val */
    QVariant setup(QString &name, QVariant v = QVariant()){

        t_setup_entry val;
        if(0 == par.ask(name, &val))  //get the config item first
            return QVariant(); //parametr of name do not exists

        if(v.isValid()){

            val.set(v.toJsonValue());  //update actual value (with all restriction applied)
            par.replace(name, val); //writeback
        }

        return val.get();
    }

    int convertf(QImage &src, void *img, unsigned free, t_campic_info *info){

        int ret = 0;
        QImage dest;

        etimer.start();

        QString f = par["pic-format"].get().toString();  //pozadovany format
        if(0 == f.compare("8bMONO")){

            dest = src.convertToFormat(QImage::Format_Indexed8);
        } else if(0 == f.compare("32bRGB")){

            dest = src.convertToFormat(QImage::Format_RGB32);
        } else { //beze zmeny - jek to prekopiruju

            dest = src;
        }

        conv_elapsed = etimer.elapsed();
        qDebug() << QString("img-conversion takes %1ms").arg(conv_elapsed);


        if((int)free < (ret = dest.byteCount())) //nemame dostatecny prostor
            return -ret;

        if(img){

            const uchar *dest_p = dest.bits();  //dest.constBits();
            memcpy(img, dest_p, ret);
        }

        /*! \todo - anotrher transformation postprocessb
         * roi & contrast etc according to manual settup (if choosen)
         */

        if(info){

            info->format = (unsigned)dest.format();
            info->w = src.width();
            info->h = src.height();
        }

        return ret;  //povedlo se
    }


public:
    /*! \brief before fist grab / for reset do original state */
    virtual int init(){

        return 0;
    }

    /*! \brief exposition control */
    virtual int64_t exposition(int64_t time, e_cam_value act = CAMVAL_UNDEF){

        Q_UNUSED(time)
        Q_UNUSED(act)
        return 0;
    }

    /*! \brief picture acquisition according to current t_vi_camera_attrib */
    int snap(void *img, unsigned free, t_campic_info *info = NULL){

        etimer.start();
        int ret = isnap(img, free, info);
        snap_elapsed = etimer.elapsed();
        qDebug() << QString("img-snapshot takes %1ms").arg(snap_elapsed);
        return ret;
    }


    /*! \brief return camera status */
    virtual e_camsta state(){
        return sta;
    }


    /*! \brief read attribute 'name' */
    QVariant get(QString &name){

        return setup(name);
    }

    /*! \brief write attribute 'name' */
    QVariant set(QString &name, QVariant v){

        return setup(name, v);
    }


    i_vi_camera_base(){

        sta = CAMSTA_UNKNOWN;
    }

    i_vi_camera_base(const QString &path):
        par(path){

        sta = CAMSTA_UNKNOWN;
    }

    virtual ~i_vi_camera_base(){;}
};

#endif // I_CAMERA_BASE

