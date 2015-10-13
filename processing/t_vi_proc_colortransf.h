#ifndef T_VI_PROC_COLORTRANSF_H
#define T_VI_PROC_COLORTRANSF_H

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


class t_vi_proc_colortransf : public i_proc_stage
{
public:
    t_vi_proc_colortransf(QString &path = QString("js_config_colortransf.txt")):
        i_proc_stage(path)
    {

    }

public slots:
    int proc(int p1, void *p2){

        p1 = p1;
        Mat *src = (Mat *)p2;
        Mat out;

        ///Convert image to gray
        cv::cvtColor(*src, out, CV_BGR2GRAY);
        *src = out;

        emit next(1, src);
        return 1;
    }

    ~t_vi_proc_colortransf();
};

#endif // T_VI_PROC_COLORTRANSF_H
