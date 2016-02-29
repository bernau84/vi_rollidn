#ifndef T_VI_CAMERA_BASLER_USB_H
#define T_VI_CAMERA_BASLER_USB_H

#include "../i_camera_base.h"

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

#if defined( USE_1394 )
// Setting for using  Basler IEEE 1394 cameras.
#include <pylon/1394/Basler1394InstantCamera.h>
typedef Pylon::CBasler1394InstantCamera Camera_t;
using namespace Basler_IIDC1394CameraParams;
#elif defined ( USE_GIGE )
// Setting for using Basler GigE cameras.
#include <pylon/gige/BaslerGigEInstantCamera.h>
typedef Pylon::CBaslerGigEInstantCamera Camera_t;
using namespace Basler_GigECameraParams;
#elif defined ( USE_CAMERALINK )
// Setting for using Basler Camera Link cameras.
#include <pylon/cameralink/BaslerCameraLinkInstantCamera.h>
typedef Pylon::CBaslerCameraLinkInstantCamera Camera_t;
using namespace Basler_CLCameraParams;
#elif defined ( USE_USB )
// Setting for using Basler USB cameras.
#include <pylon/usb/BaslerUsbInstantCamera.h>
using namespace Basler_UsbCameraParams;
#else
#error Camera type is not specified. For example, define USE_GIGE for using GigE cameras.
#endif

class t_vi_camera_basler_usb : public i_vi_camera_base
{
private:
    CBaslerUsbInstantCamera camera;

    // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
    // is initialized during the lifetime of this object.
    Pylon::PylonAutoInitTerm autoInitTerm;  /*! nevim tedy k cemu */

public:

    int init(){

        DeviceInfoList_t list;
        CTlFactory::GetInstance().EnumerateDevices(list);

        if(list.size() == 0){

            sta = CAMSTA_ERROR;
            return -1;
        }

        camera.Attach(CTlFactory::GetInstance().CreateFirstDevice());

        if(!camera.IsPylonDeviceAttached()){

            sta = CAMSTA_ERROR;
            return -2;
        }

        camera.Open();

        QString mode = par["General"].get().toString();
        if(mode.compare("MANUAL")){

            /*! \todo imprint setup to camera
                par["Width"]
                par["Height"]
                par["Gain"]
                par["Exposition"]
            */
        }

        sta = CAMSTA_PREPARED;
        return 0;
    }

    //time_us > 0 - set fix vale
    //0 - continuous
    //< 0 - once (wait for settling exposition constant to with +/- time_us tolerance)
    //returns actual exposition
    int exposure(int time_us){

        if(time_us > 0){

            /*! fixed exposition time */
            /*! \todo use setup property */
            camera.ExposureTime.SetValue(time_us);
        } else if(time_us == 0){

            camera.ExposureAuto.SetValue(ExposureAuto_Continuous);
        } else {

            camera.ExposureAuto.SetValue(ExposureAuto_Continuous);

            // When the "once" mode of operation is selected,
            // the parameter values are automatically adjusted until the related image property
            // reaches the target value. After the automatic parameter value adjustment is complete, the auto
            // function will automatically be set to "off", and the new parameter value will be applied to the
            // subsequently grabbed images.
            int n = 0, prev_exp = 2*time_us, exp = 0;
            while (abs(exp - prev_exp) > abs(time_us))
            {
                CBaslerUsbInstantCamera::GrabResultPtr_t ptrGrabResultA;
                camera.GrabOne( 5000, ptrGrabResultA);
                Pylon::DisplayImage(1, ptrGrabResultA);
                ++n;

                prev_exp = exp;
                exp = camera.ExposureTime.GetValue();

                //For demonstration purposes only. Wait until the image is shown.
                ::Sleep(100);

                qDebug() << "ExposureAuto " << n << " frames.";
                qDebug() << "Final exposure time = ";
                qDebug() << exp << " us";

                //Make sure the loop is exited.
                if (n > 100)
                {
                    throw RUNTIME_EXCEPTION( "The adjustment of auto exposure did not finish.");
                }
            }

            camera.ExposureAuto.SetValue(ExposureAuto_Off);
        }

        return camera.ExposureTime.GetValue();
    }

    int snap(void *img, unsigned free, t_campic_info *info = NULL){

        if(sta != CAMSTA_PREPARED)
            return -1;

        try
        {

            //Loading user set 1 settings - already dony in BaslerViewer
            //camera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
            //camera.UserSetLoad.Execute();

            // This smart pointer will receive the grab result data.
            CGrabResultPtr ptrGrabResult;
            camera.StartGrabbing(1);
            while ( camera.IsGrabbing())
            {
                // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
                camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

                // Image grabbed successfully?
                if (ptrGrabResult->GrabSucceeded())
                {
                    // Access the image data.
                    cout << "basler-pic-x: " << ptrGrabResult->GetWidth() << endl;
                    cout << "basler-pic-y: " << ptrGrabResult->GetHeight() << endl;
                    const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();

//#if define PYLON_WIN_BUILD && define QT_DEBUG
                    // Display the grabbed image.
                    Pylon::DisplayImage(1, ptrGrabResult);
//#endif

                    QImage src;
                    switch(ptrGrabResult->GetPixelType()){

                        case PixelType_Mono8:
                            src = QImage(pImageBuffer, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(),
                                                   QImage::Format_Indexed8);
                        break;
                        default:   //dodelat podporu rgb pokud bude potreba, bayerovych masek respektive
                            return -101;
                        break;
                    }

                    return convertf(src, img, free, info);
                }
                else
                {
                    cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
                    return 0;
                }
            }
        }
        catch (GenICam::GenericException &e)
        {
            // Error handling.
            cerr << "An exception occurred." << endl
                    << e.GetDescription() << endl;
            return -102;
        }

        return 0;
    }

    t_vi_camera_basler_usb(const QString &path):
        i_vi_camera_base(path),
        camera()
    {

    }

    t_vi_camera_basler_usb():
        camera()
    {

    }

    ~t_vi_camera_basler_usb(){;}
};

#endif // T_VI_CAMERA_BASLER_USB_H
