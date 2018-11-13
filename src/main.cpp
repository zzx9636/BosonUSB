#include "CameraStreamer.hpp"
#include "cvMatContainer.hpp"
#include "opencv2/highgui.hpp"
 
int main()
{
     //USB Camera indices
    vector<string> capture_index = { "/dev/video1", "/dev/video2" };
    bool show_img = false;
 
    //Highgui window titles
    vector<string> label;
    for (int i = 0; i < capture_index.size(); i++)
    {
        string title = "CAM_" + to_string(i);
        label.push_back(title);
    }

    //Make an instance of CameraStreamer
    CameraStreamer cam(capture_index);

    if(show_img){
        cvMatContainer* cur_container;
        cv::Mat frame;
        while(!cam.stream_stopped() && cv::waitKey(20)!=27)
        {
            for (int i = 0; i < capture_index.size(); i++){
                //Pop frame from queue and check if the frame is valid
                if (cam.frame_queue[i]->try_pop(cur_container))
                {
                    //cout<<__LINE__<<endl;
                    frame = cur_container->getImg();
                    imshow(capture_index[i], frame);
                }
            }
        }
    }else{
        cvMatContainer* cur_container0;
        cvMatContainer* cur_container1;
        while(!cam.stream_stopped())
        {
            if (cam.frame_queue[1]->try_pop(cur_container1))
            {
                cout<<cur_container1->getFrameNum()<<endl;
            }
        }
    }
    return 0;
}
