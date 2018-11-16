#include "CameraStreamer.hpp"
#include "cvMatContainer.hpp"
#include "opencv2/highgui.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>

#define MAX_DT 33334
#define NUM_CAM 2 
int main()
{
    //USB Camera indices
    vector<string> capture_index = { "/dev/video1", "/dev/video2" };
    bool show_img = false;
    const char * folder_name_0 = "./Cam_0/";
    const char * folder_name_1 = "./Cam_1/";
    char filename[60];
    unsigned long frame=1;
    
    if(capture_index.size()!=NUM_CAM){
        cout<<"Number of Cameras should be "<<NUM_CAM<<endl;
        return 0;
    }
 
    //Highgui window titles
    vector<string> label;
    for (int i = 0; i < capture_index.size(); i++)
    {
        string title = "CAM_" + to_string(i);
        label.push_back(title);
    }

    //Make an instance of CameraStreamer
    CameraStreamer cam(capture_index, RAW16_AGC, Boson_640);

    if(show_img){
        cvMatContainer* cur_container;
        cv::Mat frame;
        while(!cam.stream_stopped() && cv::waitKey(20))
        {
            for (int i = 0; i < capture_index.size(); i++){
                //Pop frame from queue and check if the frame is valid
                if (cam.frame_queue[i]->try_pop(cur_container))
                {
                    //cout<<__LINE__<<endl;
                    frame = cur_container->getImg();
                    //cout<<cur_container->getTimeCam()<<endl;
                    imshow(capture_index[i], frame);
                    delete cur_container;
                    cur_container=NULL;
                }
            }
        }
    }else{
        // create folders to record images
        struct stat info0;
        if( stat( folder_name_0, &info0 ) != -1 ){
            if( S_ISDIR(info0.st_mode) )
                printf( "%s is a directory\n", folder_name_0 );
        }
        else{
            printf( "create the directory %s\n", folder_name_0 );
            mkdir(folder_name_0, 0700);
        }

        struct stat info1;
        if( stat( folder_name_1, &info1 ) != -1 ){
            if( S_ISDIR(info1.st_mode) )
                printf( "%s is a directory\n", folder_name_1 );
        }
        else{
            printf( "create the directory %s\n", folder_name_1 );
            mkdir(folder_name_1, 0700);
        }

        //some parameters for sync
        cvMatContainer* curMat0=NULL;
        cvMatContainer* curMat1=NULL;
        int curTime0=0;
        int curTime1=0;
        
        // if camera is not stopped or there are still images in the outcomming queue
        while(!(cam.stream_stopped()) || !(cam.frame_queue[0]->empty()) || !(cam.frame_queue[1]->empty()))
        {
            if(curMat0==NULL)
                cam.frame_queue[0]->try_pop(curMat0);
            if(curMat1==NULL)
                cam.frame_queue[1]->try_pop(curMat1);
            if(curMat0 != NULL && curMat1 != NULL)
            {
                curTime0 = curMat0->getTimeCam();
                curTime1 = curMat1->getTimeCam();
                 // if both incoming stream's timestamps are less than MAX_DT
                if( abs(curTime0-curTime1) < MAX_DT)
                {
                    if(curMat0->getFFCMode()!=3 || curMat1->getFFCMode()!=3)
                        cout<<"At Frame "<<frame<<" cam1 in "<<curMat0->getFFCMode()<<" cam2 in "<<curMat1->getFFCMode()<<endl;
                    sprintf(filename, "%scam0_raw16_%lu.tiff", folder_name_0, frame);
                    if(curMat0->saveImg(filename))
                    {
                        delete curMat0;
                        curMat0 = NULL;
                    }
                    sprintf(filename, "%scam1_raw16_%lu.tiff", folder_name_1, frame);
                    if(curMat1->saveImg(filename))
                    {
                        delete curMat1;
                        curMat1 = NULL;
                    }
                    
                    frame++;

                }else if(curTime0 < curTime1)
                {
                    //cam0 is ahead. Drop frame and wait for cam1
                    delete curMat0;
                    curMat0 = NULL;
                    
                    //debug
                    /*
                    cout<<curTime0<<" "<<curTime1<<" "<<abs(curTime0-curTime1)<<endl;
                    cout<<"At frame "<<frame<<" Cam 0 is dropped"<<endl;
                    */
                }
                else{
                    delete curMat1;
                    curMat1 = NULL;
                    
                    //debug
                    /*
                    cout<<curTime0<<" "<<curTime1<<" "<<abs(curTime0-curTime1)<<endl;
                    cout<<"At frame "<<frame<<" Cam 1 is dropped"<<endl;
                    */
                }
            }
        }    
    }
    return 0;
}

