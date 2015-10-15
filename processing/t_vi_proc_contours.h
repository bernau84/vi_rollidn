#ifndef T_VI_PROC_CONTOURS_H
#define T_VI_PROC_CONTOURS_H

#include "i_proc_stage.h"


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class t_vi_proc_contours : public i_proc_stage
{
public:
    t_vi_proc_contours(QString &path = QString("js_config_contours.txt")):
        i_proc_stage(path)
    {

    }

public slots:
    int proc(int p1, void *p2){

        p1 = p1;
        Mat *src = (Mat *)p2;
        Mat out;

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        RNG rng(12345);

        findContours(*src, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

        /// Draw contours
        Mat drawing = Mat::zeros( src->size(), CV_8UC3 );
        vector<RotatedRect> minRect( contours.size() );

        for( unsigned i = 0; i < contours.size(); i++ )
           {
             if(contourArea(contours[i]) > 100){

                 Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
                 drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );

                 minRect[0] = minAreaRect( Mat(contours[i]) );
             }
           }

        Mat contours_mat;
        cv::cvtColor(drawing, contours_mat, CV_BGR2GRAY );

    //    vector<Vec4i> lines;
    //    HoughLinesP( contours_mat, lines, 1, CV_PI/180, 80, 30, 10 );
    //    for( size_t i = 0; i < lines.size(); i++ )
    //    {
    //        line( drawing, Point(lines[i][0], lines[i][1]),
    //            Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    //    }

        Point2f rect_points[4]; minRect[0].points( rect_points );
        Scalar color_line = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        for( int j = 0; j < 4; j++ )
           line( drawing, rect_points[j], rect_points[(j+1)%4], color_line, 1, 8 );

        /// Show in a window
        namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
        imshow( "Contours", drawing );

        *src = out;

        emit next(1, src);
        return 1;
    }

    ~t_vi_proc_contours(){;}
};

#endif // T_VI_PROC_CONTOURS_H
