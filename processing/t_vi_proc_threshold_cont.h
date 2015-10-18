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
private:
    vector<vector<Point> > contours;
    Mat out;

    int thresh;
    int max_thresh;
    int min_contour_area;

public:
    t_vi_proc_threshold(QString &path =  QString("js_config_threshold.txt")):
        i_proc_stage(path)
    {

        thresh = 75;    /*! \todo - from collection */
        max_thresh = 255;
        min_contour_area = 100;
    }

public slots:
    int proc(int p1, void *p2){

        p1 = p1;

        Mat *src = (Mat *)p2;

        /// Hard limit - convert to binary
        cv::threshold(*src, out, thresh, max_thresh, THRESH_BINARY);

        /// Find contours
        vector<Vec4i> hierarchy;
        findContours(out, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

        /// Draw contours
        vector<RotatedRect> minRect;
        out = Mat::zeros(src->size(), CV_8UC1);
        for(unsigned i = 0; i < contours.size(); i++)
            if(contourArea(contours[i]) > min_contour_area){

                minRect.push_back(minAreaRect(Mat(contours[i])));
                drawContours(*src, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
                drawContours(out, contours, i, Scalar(255, 255, 255), 1, 8, hierarchy, 0, Point());

                Point2f rect_points[4]; minRect.back().points(rect_points);
                for(int j = 0; j < 4; j++){

                   line(*src, rect_points[j], rect_points[(j+1)%4], Scalar(128, 0, 0), 1, 8);
                }
            }

        /// Show in a window
        cv::imshow("Threahold/Contours/BoundRect", *src);

        qDebug() << "pre_rows" << QString::number(src->rows);
        qDebug() << "pre_clms" << QString::number(src->cols);

        // matrices we'll use
        Mat M, rotated, cropped;
        Size rect_size = minRect[0].size;
        // thanks to http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
        if (minRect[0].angle < -45.) {

            minRect[0].angle += 90.0;
            swap(rect_size.width, rect_size.height);
        }
        // get the rotation matrix
        M = cv::getRotationMatrix2D(minRect[0].center, minRect[0].angle, 1.0);
        // perform the affine transformation
        cv::warpAffine(out, rotated, M, out.size(), INTER_NEAREST);
        // crop the resulting image
        cv::getRectSubPix(rotated, rect_size, minRect[0].center, cropped);

        qDebug() << "cropped_rows" << QString::number(cropped.rows);
        qDebug() << "cropped_clms" << QString::number(cropped.cols);

        out = cropped.clone();

        /// Show in a window
        /*
        cv::namedWindow("Orto/Croped", CV_WINDOW_AUTOSIZE);
        cv::imshow("Orto/Croped", out);
        */
        emit next(1, &out);
        return 1;
    }

    ~t_vi_proc_threshold(){;}
};

#endif // T_VI_PROC_THRESHOLD_H
