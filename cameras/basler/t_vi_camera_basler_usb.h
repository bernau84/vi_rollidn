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

class t_vi_camera_basler_usb : public i_vi_camera_base
{
private:
    CInstantCamera camera;

public:

    int init(){

        if(!camera.IsPylonDeviceAttached()){

            sta = CAMSTA_ERROR;
            return -1;
        }

        // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
        // is initialized during the lifetime of this object.
        Pylon::PylonAutoInitTerm autoInitTerm;  /*! nevim tedy k cemu */


        QString mode = par["General"].toString();
        if(mode.compare("MANUAL")){

            /*! \todo imprint setup to camera
                par["Width"]
                par["Height"]
                par["Gain"]
                par["Exposition"]
            */
        }

        return 0;
    }

    int snap(void *img, unsigned free, t_campic_info *info){

        try
        {
            camera.StartGrabbing(1);

            // This smart pointer will receive the grab result data.
            CGrabResultPtr ptrGrabResult;

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
                    switch(ptrGrabResult->pixelType){

                        case PixelType_Mono8:
                            src = QImage::fromData(pImageBuffer,
                                                   ptrGrabResult->GetWidth()*ptrGrabResult->GetWidth(),
                                                   QImage::Format_Indexed8);
                        break;
                        default:
                            return -1;
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
            return -1;
        }

        return 0;
    }

    t_vi_camera_basler_usb():
        camera(CTlFactory::GetInstance().CreateFirstDevice())
    {

    }

    ~t_vi_camera_basler_usb(){;}
};

#endif // T_VI_CAMERA_BASLER_USB_H
