#ifndef T_VI_CAMERA_OFFLINE_FILE_H
#define T_VI_CAMERA_OFFLINE_FILE_H

#include "../i_camera_base.h"

#include <QtWidgets/QFileDialog>

class t_vi_camera_offline_file : public i_vi_camera_base
{
public:

    int isnap(void *img, unsigned free, i_vi_camera_base::t_campic_info *info = NULL){

        QString picName = QFileDialog::getOpenFileName(NULL, "Open Image", "", "Image Files (*.png *.jpg *.bmp)");
        if(picName.isNull())
            return 0;

        QImage picFile(picName);
        if(picFile.isNull())
            return 0;

        int ret = picFile.byteCount();
        if(ret > (int)free)
            return -1;

        qDebug() << "file-pic-size:" << ret <<
                    "file-pic-x:" << picFile.width() <<
                    "file-pic-y:" << picFile.height();

        /*! convertToFormat() according to setup */
        return convertf(picFile, img, free, info);
    }

    t_vi_camera_offline_file()
    {
    }

    t_vi_camera_offline_file(const QString &path):
            i_vi_camera_base(path)
    {
    }

    virtual ~t_vi_camera_offline_file(){
    }
};

#endif // T_VI_CAMERA_OFFLINE_FILE_H
