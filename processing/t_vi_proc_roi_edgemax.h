#ifndef T_VI_PROC_ROI_EDGEMAX
#define T_VI_PROC_ROI_EDGEMAX

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


Mat src; Mat src_gray;
int thresh1 = 35;
int thresh2 = 0;
int max_thresh = 255;

/** @function thresh_callback */
void thresh_callback(int, void* )
{

  Mat output;
  Mat dst;

  GaussianBlur( src_gray, src_gray, Size(3,3), 0, 0, BORDER_DEFAULT );
  //Laplacian( src_gray, dst, CV_8U, 3, 1, 0, BORDER_DEFAULT );
  //Sobel( src_gray, dst, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT );
  //convertScaleAbs( dst, output );

  /// Detect edges using canny
  Canny( src_gray, output, thresh1, thresh2, 3 );

  int X = output.cols, Y = output.rows;
  std::vector<int> xcumsum(X);
  std::vector<int> ycumsum(Y);

  for(int x=0; x<output.cols; x++)
      for(int y=0; y<output.rows; y++)
          if(output.at<uchar>(Point(x, y))){

              xcumsum[x] += 1;
              ycumsum[y] += 1;
          }

  for(int x=0; x<output.cols; x++){

      Point p(x, xcumsum[x]);
      cv::line(src_gray, p, p, Scalar(250, 64, 64), 3, 8);
  }

  for(int y=0; y<output.rows; y++){

      Point p(output.cols - 1 - ycumsum[y], y);
      cv::line(src_gray, p, p, Scalar(250, 64, 64), 3, 8);
  }

  /// Show in a window
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
//  imshow( "Contours", drawing );
  imshow( "Contours", src_gray);
}

class t_vi_proc_roi_canny : public i_proc_stage
{

private:
    int thresh_1;
    int thresh_2;

public:
    t_vi_proc_roi_canny(QString &path =  QString(":/js_config_threshold_cont.txt")):
        i_proc_stage(path)
    {
        thresh_1 = 75;
        thresh_2 = 255;

//        thresh_1 = par["threshold_1"].get().toInt();
//        thresh_2 = par["threshold_2"].get().toInt();

        qDebug() << "Threshold 1/2:" << thresh_1 << thresh_2;

    }

public slots:
    int proc(int p1, void *p2){

        p1 = p1;

        Mat *psrc = (Mat *)p2;

        /// Convert image to gray and blur it
        //blur( *psrc, src_gray, Size(4,4) );
        src_gray = *psrc;

        /// Create Window
        namedWindow("Source", CV_WINDOW_AUTOSIZE );
        imshow("Source", *psrc );

        createTrackbar(" Canny thresh1:", "Source", &thresh1, max_thresh, thresh_callback );
        createTrackbar(" Canny thresh2:", "Source", &thresh2, max_thresh, thresh_callback );
        thresh_callback( 0, 0 );

        emit next(1, psrc);
        return 1;
    }

    ~t_vi_proc_roi_canny(){;}
};

#endif // T_VI_PROC_ROI_EDGEMAX

