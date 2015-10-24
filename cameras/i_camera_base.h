#ifndef I_CAMERA_BASE
#define I_CAMERA_BASE

#include <stdio.h>
#include <stdint.h>
#include "t_camera_attrib.h"

#include <QImage>

using namespace std;

class i_vi_camera_base {

protected:
    enum e_camsta {

        CAMSTA_UNKNOWN = 0,
        CAMSTA_PREPARED,
        CAMSTA_INPROC,
        CAMSTA_ERROR
    } sta;

    struct t_campic_info {

        unsigned h;
        unsigned w;
        unsigned format;
    };

    t_vi_camera_attrib par;     //camera attributes

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

        QString f = par["Format"].get().toString();  //pozadovany format
        if(0 == f.compare("8bMONO")){

            dest = src.convertToFormat(QImage::Format_Indexed8);
        } else if(0 == f.compare("32bRGB")){

            dest = src.convertToFormat(QImage::Format_RGB32);
        } //else - beze zmeny - jek to prekopiruju

        if((int)free < (ret = dest.byteCount())) //nemame dostatecny prostor
            return -ret;

        if(img){

            memcpy(img, src.constBits(), ret);
        }

        /*! \todo - anotrher transformation postprocess
         * roi & contrast etc according to manual settup (if choosen)
         */

        if(info){

            info->format = (unsigned)src.format();
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

    /*! \brief picture acquisition according to current t_vi_camera_attrib */
    virtual int snap(void *img, unsigned free, t_campic_info *info = NULL) = 0;


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

    virtual ~i_vi_camera_base(){;}
};

#endif // I_CAMERA_BASE

