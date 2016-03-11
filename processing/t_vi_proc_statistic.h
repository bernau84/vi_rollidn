#ifndef T_VI_PROC_STATISTIC_H
#define T_VI_PROC_STATISTIC_H


#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static const QString proc_statistic_defconfigpath(":/js_config_statistic.txt");

//vypocet pozadovaneho parametru nad obrazkem
class t_vi_proc_statistic : public i_proc_stage
{
private:
    enum t_vi_proc_statistic_ord {
        STATISTIC_NONE = 0,
        STATISTIC_MEAN,
        STATISTIC_LUMINANCE,
        STATISTIC_HIST_X,
        STATISTIC_HIST_Y,
    };

public:
    t_vi_proc_statistic(const QString &path = proc_colortransf_defconfigpath):
        i_proc_stage(path)
    {
        reload(0);
    }


    virtual ~t_vi_proc_statistic(){;}

public slots:

    int reload(int p){

        p = p;
    }

    int proc(int p1, void *p2){

        t_vi_proc_statistic_ord ord = (t_vi_proc_statistic_ord)p1;
        Mat *src = (Mat *)p2;

        switch(ord){

            case STATISTIC_MEAN:

                float res = 0.0;
                out = Mat(1, 1, CV_32FC1);
                cv::Scalar tres = cv::mean(*src);
                for(int i=0; i<tres.channels; i++)
                    res += tres[i];

                out << res/tres.channels;
            break;
            default:
            break;
        }

        emit next(1, &out);
        return 1;
    }

};
#endif // T_VI_PROC_STATISTIC_H

