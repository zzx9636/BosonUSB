#ifndef MAT_CONTAIN
#define MAT_CONTAIN

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <time.h>

using namespace std;

class cvMatContainer
{
    public:
        //constructor
        cvMatContainer(cv::Mat & input, int index, int time_stamp_camera, int time_stamp_buffer, int frame, std::vector<int> IMWRITE_PARAM);

        cvMatContainer(cv::Mat & input, int index, int time_stamp_camera, int time_stamp_buffer, int frame);
        
        cvMatContainer(cv::Mat & input);

        void assignIndex(int index);
        int getIndex() const;

        void assignTimeCam(int time_stamp);
        int getTimeCam() const;

        void assignTimeBuffer(int time_stamp);
        int getTimeBuffer() const; 

        int getFrameNum() const;

        cv::Mat getImg() const;

        bool saveImg(string path) const;

    private:
        cv::Mat img;
        int index;
        int frame_camera;
        int time_stamp_camera;
        int time_stamp_buffer;
        std::vector<int> compression_params;
};


#endif
 