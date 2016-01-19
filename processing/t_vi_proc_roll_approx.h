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
    float length;  //output result
    float diameter;
    float left_corr;
    float right_corr;

private:

    Mat out;

    Vec4f linear_approx(int from, int to){

        vector<Point> locations; Vec4f line;   // output, locations of non-zero pixels; vx, vy, x0, y0

        for(int y = from; y < to; y++)
            for(int x = 0; x < out.cols/4; x++)
                if(out.at<uchar>(Point(x, y))){

                    locations.push_back(Point(x, y));
                    break;
                }


                        for(unsigned i=0; i<locations.size(); i++){

                            Point p(locations[i].x, locations[i].y);
                            cv::line(out, p, p, Scalar(64, 64, 64), 3, 8);
                        }

        if(locations.size())
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

        int lheight = to - from;

        for(int y = from; y < to; y++)
            for(int x = diameter/4; x >= 0; x--)  //D/4 je omezeni na maximalni moznou delku kratsi poloosy elipsy
                if(out.at<uchar>(Point(x, y))){

                    locations.push_back(Point(x, y - from));  //from 0 to lheight
                    break;
                }

                            for(unsigned i=0; i<locations.size(); i++){

                                Point p(locations[i].x, locations[i].y + from);
                                cv::line(out, p, p, Scalar(192, 64, 64), 3, 8);
                            }

        Mat trans = Mat::zeros(out.size(), CV_8UC1);
        vector<Point> locations_t;
        for(unsigned i=0; i<locations.size(); i++){  //little magic - transform eliptic(quadratic) eq to linear
                                                    //with knowledge of bigger radius (height) we get

            float y_t = (1.0 * (locations[i].y - lheight/2)) / (lheight/2); //now in range <-1, +1>
            y_t = lheight/2 * sqrt(1 - pow(y_t, 2));
            Point p(locations[i].x, y_t);  //in picture coords <0, lheight>
            locations_t.push_back(p);

#ifdef QT_DEBUG
            cv::line(trans, p, p, Scalar(255, 255, 255), 2, 8);
#endif //QT_DEBUG
        }

        if(locations_t.size())
            cv::fitLine(locations_t, line, CV_DIST_L2, 0, 0.01, 0.01);

        qDebug() << "line" <<
                    "vx" << QString::number(line[0]) <<
                    "vy" << QString::number(line[1]) <<
                    "x0" << QString::number(line[2]) <<
                    "y0" << QString::number(line[3]);

#ifdef QT_DEBUG
                cv::line(trans,
                         Point(line[2]-line[0]*200, line[3]-line[1]*200),
                         Point(line[2]+line[0]*200, line[3]+line[1]*200),
                         Scalar(128, 128 ,128),
                         3, CV_AA);

                cv::line(trans,
                         Point(line[2], from),
                         Point(line[2], to),
                         Scalar(64, 64 ,64),
                         1, CV_AA);

                imshow("Trans", trans);
#endif //QT_DEBUG

        return line;
    }


    Vec4f eliptic_approx_hough(int from, int to){

        vector<Point> locations; Vec4f line;   // output, locations of non-zero pixels; vx, vy, x0, y0

        int A = (to - from) / 2;  //hlavni poloosa
        int Z = (to + from) / 2;  //umela nula

        for(int y = from; y < to; y++)
            for(int x = diameter/4; x >= 0; x--)  //D/4 je omezeni na maximalni moznou delku kratsi poloosy elipsy
                if(out.at<uchar>(Point(x, y))){

                    locations.push_back(Point(x, y));  //umela nula na stredu
                    break;
                }

                            for(unsigned i=0; i<locations.size(); i++){

                                Point p(locations[i].x, locations[i].y);
                                cv::line(out, p, p, Scalar(192, 64, 64), 3, 8);
                            }

        Mat trans = Mat::zeros(out.size(), CV_8UC1);
        vector<Point> locations_t;
        for(unsigned i=0; i<locations.size(); i++){  //little magic - transform eliptic(quadratic) eq to linear
                                                    //with knowledge of bigger radius (height) we get

            float y_t = locations[i].y - Z; //now in range <-A, A>
            y_t = A * sqrt(1 - pow(y_t/A, 2)); //<0, A>
            Point p(locations[i].x, y_t);
            locations_t.push_back(p);

#ifdef QT_DEBUG
            cv::line(trans, p, p, Scalar(255, 255, 255), 2, 8);
#endif //QT_DEBUG
        }

        if(locations_t.size()){

            cv::fitLine(locations_t, line, CV_DIST_L2, 0, 0.01, 0.01);
            vector<Vec2f> lines;
            // detect lines
            cv::HoughLines(trans, lines, 1, CV_PI/180, 150, 0, 0 );

            // draw lines
            for( size_t i = 0; i < lines.size(); i++ )
            {
                float rho = lines[i][0], theta = lines[i][1];
                Point pt1, pt2;
                double a = cos(theta), b = sin(theta);
                double x0 = a*rho, y0 = b*rho;
                pt1.x = cvRound(x0 + 1000*(-b));
                pt1.y = cvRound(y0 + 1000*(a));
                pt2.x = cvRound(x0 - 1000*(-b));
                pt2.y = cvRound(y0 - 1000*(a));
                cv::line(trans, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
            }
        }

        qDebug() << "line" <<
                    "vx" << QString::number(line[0]) <<
                    "vy" << QString::number(line[1]) <<
                    "x0" << QString::number(line[2]) <<
                    "y0" << QString::number(line[3]);

#ifdef QT_DEBUG
                cv::line(trans,
                         Point(line[2]-line[0]*200, line[3]-line[1]*200),
                         Point(line[2]+line[0]*200, line[3]+line[1]*200),
                         Scalar(128, 128 ,128),
                         3, CV_AA);

                cv::line(trans,
                         Point(line[2], from),
                         Point(line[2], to),
                         Scalar(64, 64 ,64),
                         1, CV_AA);

                imshow("Trans", trans);
#endif //QT_DEBUG

        return line;
    }

public:
    t_vi_proc_roll_approx(QString &path = QString(":/js_config_cylinder_approx.txt")):
        i_proc_stage(path)
    {


    }


    virtual ~t_vi_proc_roll_approx(){;}

public slots:
    int proc(int p1, void *p2){

        length = diameter = 0.0;
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

        diameter = out.cols - line2[2] - line1[2];
        qDebug() << "Me-Diameter:" << QString::number(diameter);

        //najdem pruseciky horni a spodni aproximace s levym a pravym celem
        //to definuje mez pro hledani elipticke aproximace cela
        float left_s1 = line1[2] - (line1[0]/(line1[1] + 1e-6))*line1[3];
        float left_s2 = line2[2] - (line2[0]/(line2[1] + 1e-6))*line2[3];
        qDebug() << "Left-Corr:" << QString::number(left_s1) << QString::number(left_s2);

        float right_s1 = line1[2] + (line1[0]/(line1[1] + 1e-6))*(out.rows - line1[3]);
        float right_s2 = line2[2] + (line2[0]/(line2[1] + 1e-6))*(out.rows - line2[3]);
        qDebug() << "Right-Corr:" << QString::number(right_s1) << QString::number(right_s2);

        if(left_s1 < line1[2]) left_s1 = line1[2];  //nejdem pod zapor ani pod stredni hodnotu aproximace
        if(left_s2 < line2[2]) left_s2 = line2[2];
        if(right_s1 < line1[2]) right_s1 = line1[2];
        if(right_s2 < line2[2]) right_s2 = line2[2];

        ///leva
        tmp = out.t(); cv::flip(tmp, out, 0); //*src;  //original
        //Vec4f line3 = eliptic_approx(line1[2], line1[2] + width); //zjednoduseno - kraj definujem jen strednim prumerem
        Vec4f line3 = eliptic_approx_hough(left_s1, out.rows - left_s2);
        double laxis = fabs(line3[3] / (line3[1] + 1e-6) * line3[0]);
        double ldia = out.rows - left_s2 - left_s1;
        double loffs = line3[2] - ((line3[3] - ldia/2) / (line3[1] + 1e-6) * line3[0]);
        qDebug() << "laxis" << QString::number(laxis);
        left_corr = loffs + laxis;  //shift calc - ie. transform from parametric to y=f(x) eq.
        qDebug() << "Correction-Left:" << QString::number(left_corr);

                cv::ellipse(out, Point(left_corr, line1[2] + diameter/2), Size(abs(laxis), abs(ldia/2)),
                        0.0, 0.0, 360, Scalar(128, 128 ,128), 2, 4);
#ifdef xQT_DEBUG
                cv::namedWindow("Left elipse", CV_WINDOW_AUTOSIZE);
                cv::imshow("Left elipse", out);
#endif //QT_DEBUG

        ///prava
        tmp = out; cv::flip(tmp, out, 1);
        Vec4f line4 = eliptic_approx_hough(right_s1, out.rows - right_s2);
        double rdia = out.rows - right_s2 - right_s1;
        double raxis = fabs((line4[3] / (line4[1] + 1e-6) * line4[0]));
        double roffs = line4[2] - ((line4[3] - ldia/2) / (line4[1] + 1e-6) * line4[0]);
        qDebug() << "raxis" << QString::number(raxis);
        right_corr = roffs + raxis;
        qDebug() << "Correction-Right:" << QString::number(right_corr);

                cv::ellipse(out, Point(right_corr, line1[2] + diameter/2), Size(abs(raxis), abs(rdia/2)),
                        0.0, 0.0, 360, Scalar(128, 128, 128), 2, 4);
                cv::namedWindow("Cylinder bases", CV_WINDOW_AUTOSIZE);
                cv::imshow("Cylinder bases", out);

        length = out.cols - left_corr - right_corr;
        qDebug() << "Height:" << QString::number(length);

        emit next(1, src);
        return 1;
    }
};

#endif // T_VI_PROC_ROLL_APPROX

