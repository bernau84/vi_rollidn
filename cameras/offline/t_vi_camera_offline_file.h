#ifndef T_VI_CAMERA_OFFLINE_FILE_H
#define T_VI_CAMERA_OFFLINE_FILE_H

#include "../i_camera_base.h"

#include <QtWidgets/QFileDialog>

class t_vi_camera_offline_file : public i_vi_camera_base
{
public:

    int snap(void *img, unsigned free){

        QString picName = QFileDialog::getOpenFileName(NULL, "Open Image", "", "Image Files (*.png *.jpg *.bmp)");
        if(picName.isNull())
            return 0;

        QImage picFile(picName);
        if(picFile.isNull())
            return 0;

        /*! \todo convertToFormat() according to setup +
         * imprint width & height back to setup */

        int ret = picFile.byteCount();
        if(ret > free)
            return -1;

        if(img)
            memcpy(img, picFile.constBits(), ret);

        return ret;
    }

    t_vi_camera_offline_file(){
    }

    ~t_vi_camera_offline_file(){
    }
};

#endif // T_VI_CAMERA_OFFLINE_FILE_H
