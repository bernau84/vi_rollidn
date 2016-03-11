#ifndef T_VI_PROC_RECTIFY
#define T_VI_PROC_RECTIFY

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/*! \todo - nechavame az budu znat kalibracni parametry
ve formatu ocv
*/

using namespace cv;
using namespace std;

static const QString proc_rectify_defconfigpath(":/js_config_rectify.txt");

class t_vi_proc_rectify : public i_proc_stage
{
private:
    Mat out;
    QList<QVariant> cam_param;
    QList<QVariant> dist_param;

public:
    t_vi_proc_rectify(const QString &path = proc_rectify_defconfigpath):
        i_proc_stage(path)
    {
        reload(0);
        qDebug() << "Rectification camera-matrix params no" << cam_param.size();
        qDebug() << "Rectification distorsion params no" << dist_param.size();
    }


    virtual ~t_vi_proc_rectify(){;}

public slots:
    int reload(int p){
        p = p;
        cam_param = par["instrict"].get().toArray().toVariantList();   //from collection
        dist_param = par["distorison"].get().toArray().toVariantList();
    }

    int proc(int p1, void *p2){

        p1 = p1;
        Mat *src = (Mat *)p2;


        emit next(1, &out);
        return 1;
    }

};

#endif // T_VI_PROC_RECTIFY

