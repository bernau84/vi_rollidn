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
        thresh = 30;
        bpath = QString("back.bmp");

        thresh = par["sub-threshold"].get().toInt();
        bpath = par["background"].get().toString();

        qDebug() << "Sub background image / diff-threshold:" << path << thresh;
    }

    virtual ~t_vi_proc_sub_backgr(){;}

public slots:
    int proc(int p1, void *p2){

        p1 = p1;
        Mat tsrc = *(Mat *)p2;

        //Mat tbck = imread(bpath.toLatin1().data());
        Mat tbck = imread("c:\\Users\\bernau84\\Documents\\sandbox\\roll_idn\\build-processing-Desktop_Qt_5_4_1_MSVC2010_OpenGL_32bit-Debug\\debug\\back.bmp");
        if(tbck.empty()){

            emit next(1, &tsrc);
            return 0;
        }

        //sjednoceni formatu
        Mat bck; cvtColor(tbck, bck, CV_BGR2RGB);
        Mat imp; cvtColor(tsrc, imp, CV_BGR2RGB);
        Mat out(imp.rows, imp.cols, CV_8UC3);

        int L1 = 32;
        int L2 = 16;

        //substract background and saturate
        //cv::absdiff(imp, bck, out);

        for(int y = 0; y < imp.rows; y++)
            for(int x = 0; x < imp.cols; x++)
                for(int r=0; r < 3; r++){ //pres barvicky

                    uchar c = imp.at<cv::Vec3b>(y,x)[r];  //mereni
                    uchar b = bck.at<cv::Vec3b>(y,x)[r];  //pozadi
                    int d = (int)c - (int)b; //vysledek

                    //baze pravidel pro odecitani pozadi
                    if(d < 0){

                        if(d < L1) d = 0; //velka zmena - pozadi svetlejsi - divne, dame 0
                        else d = -d;  //popredi je nevyznamne - berem absolutni hodnotu rozdilu

                    } else {

                        if((c > L2) && (d > L2)) d = c; //popredi vyznamne a rozdil velky - kasle na nej
                    }

                    if((d *= 2) > 255) d = 255;
                    out.at<cv::Vec3b>(y,x)[r] = d;
                }

        emit next(1, &out);
        return 1;
    }
};


#endif // T_VI_PROC_SUB_BACKGROUND
