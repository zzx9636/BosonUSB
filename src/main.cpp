#include "CameraStreamer.hpp"
#include "opencv2/highgui.hpp"
 
void main()
{
     //USB Camera indices
    vector<string> capture_index = { "/dev/video0" };
 
    //Highgui window titles
    vector<string> label;
    for (int i = 0; i < capture_index.size(); i++)
    {
        string title = "CAM_" + to_string(i);
        label.push_back(title);
    }
 
    //Make an instance of CameraStreamer
    CameraStreamer cam(capture_index);
 
    while (cv::waitKey(20) != 27)
    {
        //Retrieve frames from each camera capture thread
        for (int i = 0; i < capture_index.size(); i++)
        {
            cv::Mat frame;
            //Pop frame from queue and check if the frame is valid
            if (cam.frame_queue[i]->try_pop(frame)){
                //Show frame on Highgui window
                cv::imshow(label[i], frame);
            }
        }
    }
}
