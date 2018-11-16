#include "StereoSync.hpp"

StereoSync::StereoSync()
{
    //by default, we only save images
    this->camera_port = { "/dev/video1", "/dev/video2" };
    this->ifsave = true;
    this->ifshow = false;

    this->folder_name_0 = "./Cam_0/";
    this->folder_name_1 = "./Cam_1/";

    this->frame=1;
    Streamer = CameraStreamer(this->camera_port);

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
    Streamer = CameraStreamer(this->camera_port, ImgMode, CamType);
}
        
void setSaveDir(const char * cam1Dir, const char * cam2Dir);
void setCamType(int CamType);
void setSaveMode(int SaveMode);

void startSync();

void showStream();
void saveImage();
void ShowAndStream();