#ifndef T_VI_PROC_COLORTRANSF_H
#define T_VI_PROC_COLORTRANSF_H

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static const QString proc_colortransf_defconfigpath(":/js_config_roi_colortransf.txt");

class t_vi_proc_colortransf : public i_proc_stage
{
private:
    Mat out;
    cv::Rect roi;

public:
    t_vi_proc_colortransf(const QString &path = proc_colortransf_defconfigpath):
        i_proc_stage(path)
    {
        roi.x = roi.y = roi.width = roi.height = 0;

        roi.x = par["ROI-centerX"].get().toInt();   //from collection
        roi.y = par["ROI-centerY"].get().toInt();
        roi.width = par["ROI-width"].get().toInt();
        roi.height = par["ROI-height"].get().toInt();

        qDebug() << "ColorTr & ROI import setup:" << roi.x << roi.y << roi.width << roi.height;
    }


    virtual ~t_vi_proc_colortransf(){;}

public slots:
    int proc(int p1, void *p2){

        p1 = p1;
        Mat *src = (Mat *)p2;

        ///Convert image to gray
        cv::cvtColor(*src, out, CV_BGR2GRAY);

        ///ROI
        if(roi.width && roi.height){

            Mat cropped = out(roi);
            out = cropped.clone();   //deep copy
        }

        emit next(1, &out);
        return 1;
    }

};

#endif // T_VI_PROC_COLORTRANSF_H
