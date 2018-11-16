#ifndef CAM_STREAM
#define CAM_STREAM

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <tbb/concurrent_queue.h>
#include <opencv2/videoio.hpp>
#include <stdio.h>
#include <fcntl.h>               // open, O_RDWR
#include <opencv2/opencv.hpp>
#include <unistd.h>              // close
#include <sys/ioctl.h>           // ioctl
#include <asm/types.h>           // videodev2.h
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <time.h>

#include <cvMatContainer.hpp>
 
using namespace std;

// Define COLOR CODES
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

enum SensorType
{
    Boson_320 = 0,
    Boson_640 = 1
};
enum OutputType
{
    RAW16 = 1,
    AGC8 = 2,
    RAW16_AGC =3
};

 
class CameraStreamer{
    public:
        //Constructor for USB Camera capture
        CameraStreamer(vector<string> video_port);
        CameraStreamer(vector<string> video_port, int VideoMode, int SensorType);
        //Destructor for releasing resource(s)
        ~CameraStreamer();
        
        //this holds queue(s) which hold images from each camera
        std::vector<tbb::concurrent_queue<cvMatContainer*>*> frame_queue;

        bool stream_stopped();
        void AGC_Basic_Linear(cv::Mat input_16, cv::Mat output_8, int height, int width, unsigned int &frame, unsigned int &clock_cam, unsigned int &ffc_status); 
    
    private:

        //this holds usb camera ports
        vector<string> camera_ports;
        //this holds thread(s) which run the camera capture process
        vector<thread*> camera_thread;
        
        int camera_count;
        int VideoOutput;
        int SensorType;

        bool break_out = false; // boolean to stop camera
        std::vector<bool> stream_stop_bool;
            
        //camera handel pointers
        vector<int*> camera_handel;
        //input image buffer
        std::vector<void *> buffer_start_list;
        std::vector<struct v4l2_buffer *> bufferPtrList;

        //variable used to store image and time_stamp
        clock_t t_system;
        vector<int> img_frame_list;
        
        // To record images
        std::vector<int> compression_params;

        //raw image parameters
        int width, height;

        //8bit YCbCr image parameters
        int luma_height;
        int luma_width;

        //opencv I/O Buffer
        std::vector<cv::Mat*> thermal16List;
        std::vector<cv::Mat*> thermal16linearList;
        std::vector<cv::Mat*> thermal_lumaList;
        std::vector<cv::Mat*> thermal_RGBList;
        

    private:

        //initialize and start the camera capturing process(es)
        void startMultiCapture();
        //release all camera capture resource(s)
        void stopMultiCapture();
        //main camera capturing process which will be done by the thread(s)
        void captureFrame(int index);
};

#endif