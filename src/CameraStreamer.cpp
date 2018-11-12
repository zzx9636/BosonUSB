#include "CameraStreamer.hpp"
  
CameraStreamer::CameraStreamer(vector<string> video_port, int VideoMode=RAW16, int SensorType = Boson_640, 
    bool zoom_enable=false, bool record_enable=true, string folder_name="./Images")
{
    // initilize parameters

    this->VideoOutput = VideoMode;
    this->SensorType = SensorType;
    this->ZoomBool = zoom_enable;
    this->RecordBool = record_enable;
    this->camera_ports = video_port;
    this->camera_count = camera_ports.size();
    this->compression_params.push_back(cv::IMWRITE_PXM_BINARY);
    this->SaveDir = folder_name;

    startMultiCapture();
}
 
CameraStreamer::~CameraStreamer()
{
    stopMultiCapture();
}
 
void CameraStreamer::captureFrame(int index)
{
    cv::VideoCapture *capture = camera_capture[index];
    while (true){
        cv::Mat frame;
        //Grab frame from camera capture
        (*capture) >> frame;
        //Put frame to the queue
        frame_queue[index]->push(frame);
        //relase frame resource
        frame.release();
    }
}
 
void CameraStreamer::startMultiCapture()
{
    // set buffer parameters  
    struct v4l2_format format; 
    if(this->VideoOutput==RAW16)
    {
        printf(WHT ">>> " YEL "16 bits " WHT "capture selected\n");
        // I am requiring thermal 16 bits mode
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_Y16;
        // Select the frame SIZE (will depend on the type of sensor)
        switch (this->SensorType) {
            case Boson_320:  // Boson320
                        width=320;
                        height=256;
                        break;
                case Boson_640:  // Boson640
                        width=640;
                        height=512;
                        break;
            default:  // Boson320
                        width=320;
                        height=256;
                        break;
        }
    }else{
        printf(WHT ">>> " YEL "8 bits " WHT "YUV selected\n");
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420; // thermal, works   LUMA, full Cr, full Cb
        width = 640;
        height = 512;
    }

    // Common varibles
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;

    struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 1; 

    struct v4l2_capability* cur_cap;
    thread *t;
    tbb::concurrent_queue<cv::Mat> *q;
    for (int i = 0; i < camera_count; i++)
    {
        int * cur_handel = new int;
        //open device
        if((*cur_handel = open((camera_ports[i]).c_str(), O_RDWR)) < 0){
		    perror(RED "Error : OPEN. Invalid Video Device" WHT "\n");
		    exit(1);
	    } 
        
        // Check VideoCapture mode is available
        if(ioctl(*cur_handel, VIDIOC_QUERYCAP, cur_cap) < 0){
            perror(RED "ERROR : VIDIOC_QUERYCAP. Video Capture is not available" WHT "\n");
            exit(1);
        }
        if(!(cur_cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)){
            fprintf(stderr, RED "The device does not handle single-planar video capture." WHT "\n");
            exit(1);
        }

        // request desired FORMAT
        if(ioctl(*cur_handel, VIDIOC_S_FMT, &format) < 0){
            perror(RED "VIDIOC_S_FMT" WHT);
            exit(1);
        }

        // we need to inform the device about buffers to use.
        // and we need to allocate them.
        // we’ll use a single buffer, and map our memory using mmap.
        // All this information is sent using the VIDIOC_REQBUFS call and a
        // v4l2_requestbuffers structure:

        if(ioctl(*cur_handel, VIDIOC_REQBUFS, &bufrequest) < 0){
            perror(RED "VIDIOC_REQBUFS" WHT);
            exit(1);
        }

        struct v4l2_buffer * bufferinfo;
        memset(bufferinfo, 0, sizeof(bufferinfo));
        bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo->memory = V4L2_MEMORY_MMAP;
        bufferinfo->index = 0;

        if(ioctl(*cur_handel, VIDIOC_QUERYBUF, bufferinfo) < 0){
            perror(RED "VIDIOC_QUERYBUF" WHT);
            exit(1);
        }

        printf(WHT ">>> Buffer %i lenght=" YEL "%i" WHT "\n", i, bufferinfo->length);

        void * buffer_start = mmap(NULL, bufferinfo->length, PROT_READ | PROT_WRITE,MAP_SHARED, *cur_handel, (bufferinfo->m).offset);
        if(buffer_start == MAP_FAILED){
		    perror(RED "mmap" WHT);
		    exit(1);
	    }
        // Fill this buffer with ceros. Initialization. Optional but nice to do
	    memset(buffer_start, 0, bufferinfo->length);

        // Activate streaming
        if(ioctl(*cur_handel, VIDIOC_STREAMON, &(bufferinfo->type)) < 0){
            perror(RED "VIDIOC_STREAMON" WHT);
            exit(1);
        }

        //Put camera handel and buffer to the vector
        camera_handel.push_back(cur_handel);
        bufferPtrList.push_back(bufferinfo);
        
        //Make thread instance
        t = new thread(&CameraStreamer::captureFrame, this, i);
        
        //Put thread to the vector
        camera_thread.push_back(t);
        
        //Make a queue instance
        q = new tbb::concurrent_queue<cv::Mat>;
        
        //Put queue to the vector
        frame_queue.push_back(q);
    }
}
 
void CameraStreamer::stopMultiCapture()
{
    cv::VideoCapture *cap;
    for (int i = 0; i < camera_count; i++) {
        cap = camera_capture[i];
        if (cap->isOpened()){
        //Relase VideoCapture resource
        cap->release();
        cout << "Capture " << i << " released" << endl;
        }
    }
}