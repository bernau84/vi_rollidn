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
    t_vi_proc_colortransf(QString &path = "js_config_colortransf.txt"):
        i_proc_stage(path)
    {

    }

public slots:
    int proc(int p1, void *p2){

    }

    ~t_vi_proc_colortransf();
};

#endif // T_VI_PROC_COLORTRANSF_H
