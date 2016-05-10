#include <QCoreApplication>

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#include "t_vi_proc_roi_colortransf.h"
#include "t_vi_proc_threshold_cont.h"
#include "t_vi_proc_roll_approx.h"
#include "t_vi_proc_sub_background.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /// Load source image and convert it to gray
    //Mat src = imread( "c:\\Users\\bernau84\\turnov-meroll-daylight.bmp" );
    //Mat src = imread( "c:\\Users\\bernau84\\Documents\\sandbox\\build-roll_idn_coll-Desktop_Qt_5_4_2_MSVC2010_OpenGL_32bit-Debug\\pic29.bmp", 1 );
    //Mat src = imread( "c:\\Users\\bernau84\\Documents\\sandbox\\roll_idn\\build-processing-Desktop_Qt_5_4_1_MSVC2010_OpenGL_32bit-Debug\\debug\\smp.bmp" );
    //Mat src = imread( "c:\\Users\\bernau84\\Pictures\\test_roll_idn3.bmp");
    //Mat src = imread( "c:\\mares\\Workspace\\vi_rollidn\\processing\\test_roll_idn3.bmp");
    //Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_daybackground\\trn_meas_exp1_1_m1.bmp");
    //Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov-meroll-daylight-left-ex3-705x265mm_sel.bmp");
    //Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\roll_1300mm_floor_prec2.bmp");
    //xMat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov_fixedpos_a2_705x265_sel.bmp");
    //Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov-meroll-daylight-left-ex3-705x265mm_sel.bmp");
    //Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov_hi_150mm_1_selection_sel.bmp");
    //xMat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov-big-2-sel.bmp");
    //xMat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov_hi_70mm_lodia_5_Ldistance_sel.bmp");
    Mat src = imread("c:\\Users\\bernau84\\Pictures\\trima_precision_samples\\turnov_1300mm_1_sel.bmp");

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_roll_approx ms;
    t_vi_proc_sub_backgr bk;

    //QObject::connect(&ct, SIGNAL(next(int, void *)), &cn, SLOT(proc(int, void *)));

    QObject::connect(&bk, SIGNAL(next(int, void *)), &ct, SLOT(proc(int, void *)));
    QObject::connect(&ct, SIGNAL(next(int, void *)), &th, SLOT(proc(int, void *)));
    QObject::connect(&th, SIGNAL(next(int, void *)), &ms, SLOT(proc(int, void *)));

    bk.proc(0, &src);

    return a.exec();
}
