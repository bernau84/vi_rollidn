#ifndef T_VI_PROC_ROLL_APPROX
#define T_VI_PROC_ROLL_APPROX

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class t_vi_proc_roll_approx : public i_proc_stage
{

public:
    float width;  //output result
    float height;
    float left_corr;
    float right_corr;

private:

    Mat out;

    Vec4f linear_approx(int from, int to){

        vector<Point> locations; Vec4f line;   // output, locations of non-zero pixels; vx, vy, x0, y0

        for(int y = from; y < to; y++)
            for(int x = 0; x < out.cols; x++)
                if(out.at<uchar>(Point(x, y))){

                    locations.push_back(Point(x, y));
                    break;
                }


                        for(unsigned i=0; i<locations.size(); i++){

                            Point p(locations[i].x, locations[i].y);
                            cv::line(out, p, p, Scalar(64, 64, 64), 3, 8);
                        }

        cv::fitLine(locations, line, CV_DIST_L2, 0, 0.01, 0.01);

        qDebug() << "line" <<
                    "vx" << QString::number(line[0]) <<
                    "vy" << QString::number(line[1]) <<
                    "x0" << QString::number(line[2]) <<
                    "y0" << QString::number(line[3]);

        return line;
    }

    Vec4f eliptic_approx(int from, int to){

        vector<Point> locations; Vec4f line;   // output, locations of non-zero pixels; vx, vy, x0, y0

        int lwidth = to - from;

        for(int y = from; y < to; y++)
            for(int x = 0; x < width/4; x++)
                if(out.at<uchar>(Point(x, y))){

                    locations.push_back(Point(x, y - from));
                    break;
                }

                            for(unsigned i=0; i<locations.size(); i++){

                                Point p(locations[i].x, locations[i].y + from);
                                cv::line(out, p, p, Scalar(64, 64, 64), 3, 8);
                            }

        Mat trans = Mat::zeros(out.size(), CV_8UC1);
        vector<Point> locations_t;
        for(unsigned i=0; i<locations.size(); i++){  //little magic - transform eliptic(quadratic) eq to linear
                                                    //with knowledge of bigger radius (width) we get

            float y_t = locations[i].y - lwidth/2; //now in range +/-width/2
            Point p(locations[i].x, lwidth * sqrt(1 - pow(y_t/(lwidth/2), 2)));  //in picture coords
            locations_t.push_back(p);

#ifdef xQT_DEBUG
            cv::line(trans, p, p, Scalar(255, 255, 255), 2, 8);
#endif //QT_DEBUG
        }

        cv::fitLine(locations_t, line, CV_DIST_L2, 0, 0.01, 0.01);

        qDebug() << "line" <<
                    "vx" << QString::number(line[0]) <<
                    "vy" << QString::number(line[1]) <<
                    "x0" << QString::number(line[2]) <<
                    "y0" << QString::number(line[3]);

#ifdef xQT_DEBUG
                cv::line(trans,
                         Point(line[2]-line[0]*200, line[3]-line[1]*200),
                         Point(line[2]+line[0]*200, line[3]+line[1]*200),
                         Scalar(128, 128 ,128),
                         3, CV_AA);

                imshow("Trans", trans);
#endif //QT_DEBUG

        return line;
    }

public:
    t_vi_proc_roll_approx(QString &path = QString(":/js_config_cylinder_approx.txt")):
        i_proc_stage(path)
    {


    }

public slots:
    int proc(int p1, void *p2){

        width = height = 0.0;
        left_corr = right_corr = 0.0;

        p1 = p1;
        Mat *src = (Mat *)p2;

        ///horni strana
        cv::transpose(*src, out);
        Vec4f line1 = linear_approx(out.cols/3, out.rows - out.cols/3);

                cv::line(out,
                         Point(line1[2]-line1[0]*100, line1[3]-line1[1]*100),
                         Point(line1[2]+line1[0]*100, line1[3]+line1[1]*100),
                         Scalar(128, 128 ,128),
                         2, CV_AA);

#ifdef xQT_DEBUG
                cv::namedWindow("Transposed", CV_WINDOW_AUTOSIZE);
                cv::imshow("Transposed", out);
#endif // QT_DEBUG

        ///spodni strana
        cv::Mat tmp = out; cv::flip(tmp, out, 1);
        Vec4f line2 = linear_approx(out.cols/3, out.rows - out.cols/3);

                cv::line(out,
                         Point(line2[2]-line2[0]*100, line2[3]-line2[1]*100),
                         Point(line2[2]+line2[0]*100, line2[3]+line2[1]*100),
                         Scalar(128, 128 ,128),
                         2, CV_AA);
#ifdef xQT_DEBUG
                cv::namedWindow("Cylinder diameter", CV_WINDOW_AUTOSIZE);
                cv::imshow("Cylinder diameter", out.t());
#endif // QT_DEBUG

        width = out.cols - line2[2] - line1[2];
        qDebug() << "Me-Width:" << QString::number(width);

        //najdem pruseciky horni a spodni aproximace s levym a pravym celem
        //to definuje mez pro hledani elipticke aproximace cela
        float left_s1 = line1[2] - (line1[0]/line1[1])*line1[3];
        float left_s2 = line2[2] - (line2[0]/line2[1])*line2[3];
        qDebug() << "Left-Corr:" << QString::number(left_s1) << QString::number(left_s2);

        float right_s1 = line1[2] + (line1[0]/line1[1])*(out.rows - line1[3]);
        float right_s2 = line2[2] + (line2[0]/line2[1])*(out.rows - line2[3]);
        qDebug() << "Right-Corr:" << QString::number(right_s1) << QString::number(right_s2);

        if(left_s1 < line1[2]) left_s1 = line1[2];  //nejdem pod zapor ani pod stredni hodnotu aproximace
        if(left_s2 < line2[2]) left_s2 = line2[2];
        if(right_s1 < line1[2]) right_s1 = line1[2];
        if(right_s2 < line2[2]) right_s2 = line2[2];

        ///leva
        tmp = out.t(); cv::flip(tmp, out, 0); //*src;  //original
        //Vec4f line3 = eliptic_approx(line1[2], line1[2] + width); //zjednoduseno - kraj definujem jen strednim prumerem
        Vec4f line3 = eliptic_approx(left_s1, out.rows - left_s2);
        left_corr = fabs((line3[3] / line3[1]) * line3[0]) + line3[2];  //shift calc - ie. transform from parametric to y=f(x) eq.
        qDebug() << "Correction-Left:" << QString::number(left_corr);

                cv::ellipse(out, Point(left_corr, line1[2] + width/2), Size(left_corr, width/2),
                        0.0, 0.0, 360, Scalar(128, 128 ,128), 2, 4);
#ifdef xQT_DEBUG
                cv::namedWindow("Left elipse", CV_WINDOW_AUTOSIZE);
                cv::imshow("Left elipse", out);
#endif //QT_DEBUG

        ///prava
        tmp = out; cv::flip(tmp, out, 1);
        Vec4f line4 = eliptic_approx(right_s1, out.rows - right_s2);
        right_corr = fabs((line4[3] / line4[1]) * line4[0]) + line4[2];
        qDebug() << "Correction-Right:" << QString::number(right_corr);

                cv::ellipse(out, Point(right_corr, line1[2] + width/2), Size(right_corr, width/2),
                        0.0, 0.0, 360, Scalar(128, 128 ,128), 2, 4);
                cv::namedWindow("Cylinder bases", CV_WINDOW_AUTOSIZE);
                cv::imshow("Cylinder bases", out);

        height = out.cols - left_corr - right_corr;
        qDebug() << "Height:" << QString::number(height);

        emit next(1, src);
        return 1;
    }

    ~t_vi_proc_roll_approx(){;}
};

#endif // T_VI_PROC_ROLL_APPROX

