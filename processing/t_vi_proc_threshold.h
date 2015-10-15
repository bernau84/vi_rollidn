#ifndef T_VI_PROC_THRESHOLD_H
#define T_VI_PROC_THRESHOLD_H


#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


class t_vi_proc_threshold : public i_proc_stage
{
public:
    t_vi_proc_threshold(QString &path =  QString("js_config_threshold.txt")):
        i_proc_stage(path)
    {

    }

public slots:
    int proc(int p1, void *p2){

        p1 = p1;
        Mat *src = (Mat *)p2;
        Mat out;

        int thresh = 100;    /*! \todo - from collection */
        int max_thresh = 255;

        threshold( *src, out, thresh, max_thresh, THRESH_BINARY);
        namedWindow( "Thr", CV_WINDOW_AUTOSIZE );
        imshow( "Thr", out );

        *src = out;

        emit next(1, src);
        return 1;
    }

    ~t_vi_proc_threshold(){;}
};

#endif // T_VI_PROC_THRESHOLD_H
