#include "StereoSync.hpp"

StereoSync::StereoSync()
{
    //by default, we only save images
    this->camera_port = { "/dev/video1", "/dev/video2" };
    Streamer.AssignPort(this->camera_port);
    //by default, we output RAW16 with Boson640 camera
    Streamer.AssignOuputMode(RAW16);
    Streamer.AssignSensortype(Boson_640);

    this->ifsave = true;
    this->ifshow = false;

    this->folder_name_0 = "./Cam_0/";
    this->folder_name_1 = "./Cam_1/";

    this->frame=1;
    Streamer = CameraStreamer(this->camera_port);
    //Initialize the buffer and start capture
    Streamer.startMultiCapture();

}

StereoSync::StereoSync(vector<string> camera_port, int CamType, int ImgMode, bool ifRecord, bool ifShow)
{
    this->camera_port=camera_port;
    assert(camera_port.size()==NUM_CAM);
    this->ifsave = ifRecord;
    this->ifshow = ifShow;

    this->folder_name_0 = "./Cam_0/";
    this->folder_name_1 = "./Cam_1/";

    this->frame=1;

    if(this->ifshow)
    {
        for (int i = 0; i < NUM_CAM; i++)
        {
            string title = "CAM_" + to_string(i);
            label.push_back(title);
        }
    }
    Streamer.AssignPort(this->camera_port);
    Streamer.AssignOuputMode(ImgMode);
    Streamer.AssignSensortype(CamType);
    //Initialize the buffer and start capture
    Streamer.startMultiCapture();
    
}
        
void StereoSync::setSaveDir(const char * cam1Dir, const char * cam2Dir)
{
    //folder has to be in the root
    this->folder_name_0 = cam2Dir;
    this->folder_name_1 = cam1Dir;
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

}

void StereoSync::startSync()
{
    if(this->ifsave && this->ifshow)
    {
        ShowAndStream();
    }
    else if(this->ifsave){
        saveImage();
    }else{
        showStream();
    }
}

void StereoSync::showStream()
{
    while(!Streamer.stream_stopped() && cv::waitKey(20))
    {
       
        //Pop frame from queue and check if the frame is valid
        if (Streamer.frame_queue[0]->try_pop(curMat0))
        {
            imshow(label[0], curMat0->getImg());
            delete curMat0;
            curMat0=NULL;
        }
        if (Streamer.frame_queue[1]->try_pop(curMat1))
        {
            imshow(label[1], curMat1->getImg());
            delete curMat1;
            curMat1=NULL;
        }
    }
}


void saveImage();
void ShowAndStream();