#ifndef T_VI_PROC_SUB_BACKGROUND
#define T_VI_PROC_SUB_BACKGROUND

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QString>

using namespace cv;
using namespace std;

class t_vi_proc_sub_backgr : public i_proc_stage
{

private:
    int thresh;
    QString bpath;

public:
    t_vi_proc_sub_backgr(QString &path =  QString(":/js_config_sub_background.txt")):
        i_proc_stage(path)
    {
        thresh = 20;
        bpath = QString("back.bmp");

        thresh = par["sub-threshold"].get().toInt();
        bpath = par["background"].get().toString();

        qDebug() << "Sub background image / diff-threshold:" << path << thresh;
    }

    virtual ~t_vi_proc_sub_backgr(){;}

public slots:
    int proc(int p1, void *p2){

        p1 = p1;

        Mat *psrc = (Mat *)p2;
        //Mat bck = imread(bpath.toLatin1().data());
        Mat tbck = imread("c:\\Users\\bernau84\\Documents\\sandbox\\roll_idn\\build-processing-Desktop_Qt_5_4_1_MSVC2010_OpenGL_32bit-Debug\\debug\\back.bmp");
        Mat bck; cvtColor(tbck, bck, CV_BGR2BGRA);
        Mat imp; cvtColor(*psrc, imp, CV_BGR2BGRA);
        Mat out;

        qDebug() << "bck: " << bck.elemSize();
        qDebug() << "src: " << psrc->elemSize();
        qDebug() << "img: " << imp.elemSize();

        if(bck.empty()){

            emit next(1, psrc);
            return 0;
        }

        //substract background and saturate
        cv::absdiff(imp, bck, out);

        //hard limit - convert to binary
        cv::threshold(out, bck, 100/*thresh*/, 255, THRESH_BINARY);
        out = bck;

        //create window
        namedWindow("Threshold substracted background", CV_WINDOW_AUTOSIZE );
        imshow("Threshold substracted background", out );

        emit next(1, &out);
        return 1;
    }
};


#endif // T_VI_PROC_SUB_BACKGROUND

