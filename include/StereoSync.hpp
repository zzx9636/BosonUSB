#ifndef Sync
#define Sync

#include "CameraStreamer.hpp"
#include "cvMatContainer.hpp"
#include "opencv2/highgui.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>
#define NUM_CAM 2 
#define MAX_DT 33334

class StereoSync
{
    public:
        //constructor
        StereoSync();
        StereoSync(vector<string> camera_port, int CamType, int ImgMode, bool ifRecord, bool ifShow);
        
        //set parameter
        void setSaveDir(const char * cam1Dir, const char * cam2Dir);
          
        void startSync();


    private:
        // main streamer
        CameraStreamer Streamer;

        //file directory
        vector<string> camera_port;
        const char * folder_name_0;
        const char * folder_name_1;
        char filename[60];

        unsigned long frame=1;

        bool ifshow;
        bool ifsave;

        //Highgui window titles
        vector<string> label;

        cvMatContainer* curMat0=NULL;
        cvMatContainer* curMat1=NULL;
        int curTime0=0;
        int curTime1=0;

        void showStream();
        void saveImage();
        void ShowAndStream();

};



#endif