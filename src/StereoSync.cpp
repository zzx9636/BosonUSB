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
}

void StereoSync::startSync()
{
    if(this->ifsave && this->ifshow){
        setupLog();
        //call main loop
        SaveAndStream();
    }
    else if(this->ifsave){
        setupLog();
        //call main loop
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
        char c = cv::waitKey(0);
        if ('q' == c)
            Streamer.requestQuit(true);
    }
}


void StereoSync::SaveAndStream()
{
    while(!(Streamer.stream_stopped()) || !(Streamer.frame_queue[0]->empty()) || !(Streamer.frame_queue[1]->empty()))
    {
        //pop only if we have processed last image
        if(curMat0==NULL)
            Streamer.frame_queue[0]->try_pop(curMat0);
        if(curMat1==NULL)
            Streamer.frame_queue[1]->try_pop(curMat1);

        //make sure we have image from both left and right camera
        if(curMat0 != NULL && curMat1 != NULL)
        {
            curTime0 = curMat0->getTimeCam();
            curTime1 = curMat1->getTimeCam();
            // if both incoming stream's timestamps are less than MAX_DT
            if( abs(curTime0-curTime1) < MAX_DT)
            {
                sprintf(filename, "%scam0_raw16_%lu.tiff", folder_name_0, frame);
                if(curMat0->saveImg(filename))
                {
                    imshow(label[0], curMat0->getImg());
                    delete curMat0;
                    curMat0 = NULL;
                }
                sprintf(filename, "%scam1_raw16_%lu.tiff", folder_name_1, frame);
                if(curMat1->saveImg(filename))
                {
                    imshow(label[1], curMat1->getImg());
                    delete curMat1;
                    curMat1 = NULL;
                }
                frame++;
                cv::waitKey(30);

            }else if(curTime0 < curTime1)
            {
                 //cam0 is ahead. Drop frame and wait for new cam0 to catch up with cam1
                delete curMat0;
                curMat0 = NULL;
            }
            else{
                //cam1 is ahead. Drop frame and wait for new cam1 to catch up with cam0
                delete curMat1;
                curMat1 = NULL;
            }
        }
        char c = cv::waitKey(1);
        if ('q' == c)
            Streamer.requestQuit(true);
    }
}

void StereoSync::saveImage()
{
    while(!(Streamer.stream_stopped()) || !(Streamer.frame_queue[0]->empty()) || !(Streamer.frame_queue[1]->empty()))
    {
        //pop only if we have processed last image
        if(curMat0==NULL)
            Streamer.frame_queue[0]->try_pop(curMat0);
        if(curMat1==NULL)
            Streamer.frame_queue[1]->try_pop(curMat1);

        //make sure we have image from both left and right camera
        if(curMat0 != NULL && curMat1 != NULL)
        {
            //get time stamps from camera
            curTime0 = curMat0->getTimeCam();
            curTime1 = curMat1->getTimeCam();
            // if both incoming stream's timestamps are less than MAX_DT
            if( abs(curTime0-curTime1) < MAX_DT)
            {
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
                //cam0 is ahead. Drop frame and wait for new cam0 to catch up with cam1
                delete curMat0;
                curMat0 = NULL;
            }
            else{
                //cam1 is ahead. Drop frame and wait for new cam1 to catch up with cam0
                delete curMat1;
                curMat1 = NULL;
            }
        }
        char c = cv::waitKey(1);
        if ('q' == c)
            Streamer.requestQuit(true);
    }
}

void StereoSync::setupLog()
{
    //get current time
    char buff[20];
    struct tm *sTm;
    time_t now = time (0);
    sTm = gmtime (&now);
    strftime (buff, sizeof(buff), "%Y_%m_%d_%H_%M_%S", sTm);
    
    //create log root
    char root_dir[60];
    sprintf(root_dir, "./recording_%s", buff);
    mkdir(root_dir,0700);
    chdir(root_dir);
    printf(WHT ">>> Folder " YEL "%s" WHT " selected to record files\n", root_dir);
    char logname[60];
    sprintf(logname, "./log_%s.txt", buff);
    logfile.open(logname);

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