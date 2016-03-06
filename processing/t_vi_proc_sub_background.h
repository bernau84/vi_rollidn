#ifndef T_VI_PROC_SUB_BACKGROUND
#define T_VI_PROC_SUB_BACKGROUND

#include "i_proc_stage.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QString>

using namespace cv;
using namespace std;

static const QString proc_sub_backgr_defconfigpath(":/js_config_sub_background.txt");

class t_vi_proc_sub_backgr : public i_proc_stage
{

private:
    int bck_lighter_thresh;
    int bck_darker_thresh;

    QString bpath;

    int bck_avr;
    int bck_num;
public:

    enum t_vi_proc_sub_backgr_ord {
        SUBBCK_RESET = 0,
        SUBBCK_REFRESH = 1,
        SUBBCK_SUBSTRACT = 2
    };

    t_vi_proc_sub_backgr(const QString &path = proc_sub_backgr_defconfigpath):
        i_proc_stage(path)
    {

        bck_num = 0;

        //pozadi svetlejsi nez L1 je nahrazeno 0
        if(0 >= (bck_lighter_thresh = par["bck-ligher-threshold"].get().toInt()))
            bck_lighter_thresh = 32;

        //popredi svetlejsi nez L2 zustava nezmeneno
        if(0 >= (bck_darker_thresh = par["bck-darker-threshold"].get().toInt()))
            bck_darker_thresh = 16;

        //prumerovani pozadi
        if(0 >= (bck_avr = par["background-avr"].get().toInt()))
            bck_avr = 1;

        //cesta ke snimku pozadi
        bpath = par["background"].get().toString();
        if(bpath.isEmpty()) bpath = QString("back.bmp");

        qDebug() << "Sub background image / light-threshold / dark-threshold:" << bpath << bck_lighter_thresh << bck_darker_thresh;
    }

    virtual ~t_vi_proc_sub_backgr(){;}

public slots:
    int proc(int p1, void *p2){

        //const char *std_bpath = "c:\\Users\\bernau84\\Documents\\sandbox\\roll_idn\\build-processing-Desktop_Qt_5_4_1_MSVC2010_OpenGL_32bit-Debug\\debug\\back.bmp";
        //const char *std_bpath = "c:\\Users\\bernau84\\Pictures\\trima_daybackground\\trn_bck_exp1_1.bmp";

        QByteArray ba_bpath= bpath.toLatin1();
        const char *std_bpath = ba_bpath.data();

        Mat tbck = imread(std_bpath);
        Mat tsrc = *(Mat *)p2;
        int tord = (t_vi_proc_sub_backgr_ord)p1;

        if(tord == SUBBCK_RESET){

            qDebug() << "t_vi_proc_sub_background averaging reset.";
            bck_num = 0;
            return 0;
        } else if(tsrc.empty()){

            qDebug() << "t_vi_proc_sub_background empty input!";
            return 0;  //neni co pocitat
        }

        //sjednoceni formatu
        Mat inp; cvtColor(tsrc, inp, CV_BGR2RGB);
        Mat bck; //cvtColor(tbck, bck, CV_BGR2RGB);

        switch(tord){

            case SUBBCK_REFRESH:

                //nekdo nam smazal pozdi - zacnem prumerovat znovu
                //nebo byl zadav externi povel nebo to nema smysl protoze prumerovani je vypnute
                if((bck_avr < 2) || (bck_num == 0) || (tbck.empty())){

                    bck = inp;
                } else {

                    //todo - nefunguje, svetle plochy postupne saturuji
                    //otazka jestli to budem moct pouzit s promennou ecpozici
                    //stejne ne
                    cvtColor(tbck, bck, CV_BGR2RGB);  //stejny format jako imp!
                    Mat dif(inp.rows, inp.cols, CV_32SC3);
                    Mat sbck(inp.rows, inp.cols, CV_32SC3); sbck = bck;
                    subtract(sbck, inp, dif);  //rozdil potrebujem znamenkovy
                    dif /= bck_avr;
                    sbck += dif; //pak bude prumerovani fungovat
                    bck = sbck;
                }

                imwrite(std_bpath, bck);
                imshow("Backround cummulative", bck);
                return ++bck_num;  //konec signal dal nesirime

            default:  //normalni mode

                if(tbck.empty()){

                    emit next(1, &tsrc);
                    return 0;
                }

                cvtColor(tbck, bck, CV_BGR2RGB);
                break;
        }


        Mat out(inp.rows, inp.cols, CV_8UC3);

        int L1 = bck_lighter_thresh;  //pozadi svetlejsi nez L1 je nahrazeno 0
        int L2 = bck_darker_thresh;  //popredi svetlejsi nez L2 zustava nezmeneno

        //substract background and saturate
        //cv::absdiff(imp, bck, out);

        for(int y = 0; y < inp.rows; y++)
            for(int x = 0; x < inp.cols; x++)
                for(int r=0; r < 3; r++){ //pres barvicky

                    uchar c = inp.at<cv::Vec3b>(y,x)[r];  //mereni
                    uchar b = bck.at<cv::Vec3b>(y,x)[r];  //pozadi
                    int d = (int)c - (int)b; //vysledek

                    //baze pravidel pro odecitani pozadi
                    if(d < 0){

                        if(d < L1) d = 0; //velka zmena - pozadi svetlejsi - divne, dame 0
                        else d = -d;  //popredi je nevyznamne - berem absolutni hodnotu rozdilu

                    } else {

                        if((c > L2) && (d > L2)) d = c; //popredi vyznamne a rozdil velky - kasle na nej
                    }

                    if((d *= 2) > 255) d = 255;
                    out.at<cv::Vec3b>(y,x)[r] = d;
                }

        emit next(1, &out);
        return 1;
    }
};


#endif // T_VI_PROC_SUB_BACKGROUND

