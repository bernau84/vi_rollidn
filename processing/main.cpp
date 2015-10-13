#include "mainwindow.h"
#include <QApplication>

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#include "t_vi_proc_colortransf.h"
#include "t_vi_proc_contours.h"
#include "t_vi_proc_threshold.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    /// Load source image and convert it to gray
    Mat src = imread( "c:\\Users\\bernau84\\Pictures\\role2_detail_retus_1_2.bmp", 1 );

    t_vi_proc_colortransf ct;
    t_vi_proc_threshold th;
    t_vi_proc_contours cn;

    QObject::connect(&ct, SIGNAL(next(int, void *)), &th, SLOT(proc(int, void *)));
    QObject::connect(&th, SIGNAL(next(int, void *)), &cn, SLOT(proc(int, void *)));

    ct.proc(0, &src);

    return a.exec();
}
