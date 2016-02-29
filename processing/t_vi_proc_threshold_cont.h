#ifndef T_VI_PROC_THRESHOLD_H
#define T_VI_PROC_THRESHOLD_H


#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static const QString proc_threshold_defconfigpath(":/js_config_threshold_cont.txt");

class t_vi_proc_threshold : public i_proc_stage
{
public:
    RotatedRect maxContRect;  //expotni aby sem si mohl zkontrolovat vysledky

private:
    vector<vector<Point> > contours;
    Mat out;

    int thresh;
    int max_thresh;
    int min_contour_area;

public:
    t_vi_proc_threshold(const QString &path = proc_threshold_defconfigpath):
        i_proc_stage(path)
    {
        thresh = 90;    /*! \todo - from collection */
        max_thresh = 255;
        min_contour_area = 100;

        thresh = par["threshold_positive"].get().toInt();
        max_thresh = par["threshold_binaryval"].get().toInt();
        min_contour_area = par["contour_minimal"].get().toInt();

        qDebug() << "Threshold & contours setup:" << thresh << max_thresh << min_contour_area;
    }


    virtual ~t_vi_proc_threshold(){;}

public slots:
    int proc(int p1, void *p2){

        p1 = p1;

        Mat *src = (Mat *)p2;

        maxContRect = RotatedRect(Point2f(0, 0), Size2f(0, 0), 0.0);

        /// Hard limit - convert to binary
        cv::threshold(*src, out, thresh, max_thresh, THRESH_BINARY);

        /// Find contours
        vector<Vec4i> hierarchy;
        findContours(out, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

        /// Draw contours
        int maxarea = 0, maxindex = 0;    ///
        RotatedRect crect;
        out = Mat::zeros(src->size(), CV_8UC1);
        for(unsigned i = 0; i < contours.size(); i++){

            int area = contourArea(contours[i]);
            if(area > min_contour_area){

                crect = minAreaRect(Mat(contours[i]));
                drawContours(*src, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());

                Point2f rect_points[4]; crect.points(rect_points);
                for(int j = 0; j < 4; j++){

                   line(*src, rect_points[j], rect_points[(j+1)%4], Scalar(128, 0, 0), 1, 8);
                }

                if(maxarea < area){

                    maxarea = area; maxindex = i;
                    maxContRect = crect;
                }
            }
        }

        drawContours(out, contours, maxindex, Scalar(255, 255, 255), 1, 8, hierarchy, 0, Point());

        /// Show in a window
        Mat resized;
        resize(*src, resized, Size(), 0.5, 0.5);
        cv::namedWindow("Contours/BoundRect", CV_WINDOW_AUTOSIZE);
        cv::imshow("Contours/BoundRect", resized);
        cv::resizeWindow("Contours/BoundRect", resized.cols, resized.rows);

        qDebug() << "pre_rows" << QString::number(src->rows);
        qDebug() << "pre_clms" << QString::number(src->cols);

        // matrices we'll use
        Mat M, rotated, cropped;
        Size rect_size = maxContRect.size;

        // thanks to http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
        if (maxContRect.angle < -45.) {

            maxContRect.angle += 90.0;
            swap(rect_size.width, rect_size.height);
        }

        // get the rotation matrix
        M = cv::getRotationMatrix2D(maxContRect.center,
                                    maxContRect.angle, 1.0);

        // perform the affine transformation
        cv::warpAffine(out, rotated, M, out.size(), INTER_NEAREST);

        // crop the resulting image
        cv::getRectSubPix(rotated, rect_size,
                          maxContRect.center, cropped);

        qDebug() << "cropped_rows" << QString::number(cropped.rows);
        qDebug() << "cropped_clms" << QString::number(cropped.cols);

        out = cropped.clone();

//        /// Show in a window
//        cv::namedWindow("Orto/Croped", CV_WINDOW_AUTOSIZE);
//        cv::imshow("Orto/Croped", out);
//        //cv::resizeWindow("Orto/Croped", out.cols, out.rows);

        emit next(1, &out);
        return 1;
    }

};

#endif // T_VI_PROC_THRESHOLD_H
