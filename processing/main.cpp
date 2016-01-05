#include <QCoreApplication>

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#include "t_vi_proc_roi_grayscale.h"
#include "t_vi_proc_roi_edgemax.h"
#include "t_vi_proc_threshold_cont.h"
#include "t_vi_proc_roll_approx.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /// Load source image and convert it to gray
    Mat src = imread( "c:\\Users\\bernau84\\Documents\\sandbox\\build-roll_idn_coll-Desktop_Qt_5_4_2_MSVC2010_OpenGL_32bit-Debug\\pic29.bmp", 1 );
    //Mat src = imread( "c:\\Users\\bernau84\\Pictures\\trima_demo\\turnov_hi_70mm_lodia_3_Ldistance_selection.bmp" );

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_roll_approx ms;
    t_vi_proc_roi_canny cn;

    QObject::connect(&ct, SIGNAL(next(int, void *)), &cn, SLOT(proc(int, void *)));
    //QObject::connect(&th, SIGNAL(next(int, void *)), &ms, SLOT(proc(int, void *)));

    ct.proc(0, &src);

    return a.exec();
}
