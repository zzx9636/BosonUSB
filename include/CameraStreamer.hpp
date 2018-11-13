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
    AGC8 = 2
};

 
class CameraStreamer{
public:
    //this holds usb camera indices
    vector<string> camera_ports;
    //this holds OpenCV VideoCapture pointers
    vector<cv::VideoCapture*> camera_capture;
    //this holds queue(s) which hold images from each camera
    std::vector<tbb::concurrent_queue<cvMatContainer*>*> frame_queue;
    //this holds thread(s) which run the camera capture process
    vector<thread*> camera_thread;
    
    //Constructor for USB Camera capture
    CameraStreamer(vector<string> video_port);
    CameraStreamer(vector<string> video_port, int VideoMode, int SensorType, bool zoom_enable, bool record_enable, string folder_name);
    //Destructor for releasing resource(s)
    ~CameraStreamer();
    bool stream_stopped();
    void AGC_Basic_Linear(cv::Mat input_16, cv::Mat output_8, int height, int width, unsigned int &frame, unsigned int &clock_cam, unsigned int &ffc_status); 
 
private:
    int camera_count;
    int VideoOutput;
    int SensorType;
    bool RecordBool;
    bool ZoomBool;
    bool break_out = false; // boolean to stop camera
    
    //variable used to store image and time_stamp
    clock_t t_system;
    vector<int> img_index_list;
    
    //folder to save images
    string SaveDir;

    // To record images
	std::vector<int> compression_params;

    //image parameters
    int width, height;

    //camera handel pointers
    vector<int*> camera_handel;
    //input image buffer
    std::vector<void *> buffer_start_list;
    std::vector<struct v4l2_buffer *> bufferPtrList;

    //opencv input Buffer
    std::vector<cv::Mat*> thermal16List;
    std::vector<cv::Mat*> thermal16linearList;

    //initialize and start the camera capturing process(es)
    void startMultiCapture();
    //release all camera capture resource(s)
    void stopMultiCapture();
    //main camera capturing process which will be done by the thread(s)
    void captureFrame(int index);


};

#endif