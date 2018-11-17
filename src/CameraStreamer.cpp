#include "CameraStreamer.hpp"
CameraStreamer::CameraStreamer()
{
    this->VideoOutput = RAW16;
    this->SensorType = Boson_640;
}

CameraStreamer::CameraStreamer(vector<string> video_port)
{
    // initilize parameters
    this->VideoOutput = RAW16;
    this->SensorType = Boson_640;
    this->camera_ports = video_port;
    this->camera_count = camera_ports.size();
    cout<<"Total "<<camera_count<<" camera found"<<endl;
    this->compression_params.push_back(cv::IMWRITE_PXM_BINARY);

    //startMultiCapture();
}


CameraStreamer::CameraStreamer(vector<string> video_port, int VideoMode=RAW16, int SensorType = Boson_640)
{
    // initilize parameters

    this->VideoOutput = VideoMode;
    this->SensorType = SensorType;
    this->camera_ports = video_port;
    this->camera_count = camera_ports.size();
    cout<<"Total "<<camera_count<<" camera found"<<endl;
    this->compression_params.push_back(cv::IMWRITE_PXM_BINARY);

    //startMultiCapture();
}
 
CameraStreamer::~CameraStreamer()
{
    stopMultiCapture();
}

void CameraStreamer::AssignPort(vector<string> video_port)
{
    this->camera_ports = video_port;
    this->camera_count = this->camera_ports.size();
}

void CameraStreamer::AssignSensortype(int SensorType)
{
    this->SensorType = SensorType;
}

void CameraStreamer::AssignOuputMode(int VideoMode)
{
    this->VideoOutput = VideoMode;
}
 
void CameraStreamer::captureFrame(int index)
{
    int * camera_handel_cur = this->camera_handel[index];
    struct v4l2_buffer * bufferinfo = this->bufferPtrList[index];
    
    cv::Mat* thermal16 = this->thermal16List[index];
    cv::Mat* thermal16_linear = this->thermal16linearList[index];
    cv::Mat* thermal_luma = this->thermal_lumaList[index];
    cv::Mat* thermal_RGB = this->thermal_RGBList[index];

    cvMatContainer* img2queue = NULL;

    unsigned int camera_clock, camera_frame, FFC_mode;  
    
    while(!break_out){
        // The buffer's waiting in the incoming queue.
        if(ioctl(*camera_handel_cur, VIDIOC_QBUF, bufferinfo) < 0){
            perror(RED "VIDIOC_QBUF" WHT);
            exit(1);
        }
        // The buffer's waiting in the outgoing queue.
        if(ioctl(*camera_handel_cur, VIDIOC_DQBUF, bufferinfo) < 0) {
            perror(RED "VIDIOC_QBUF" WHT);
            exit(1);
        }
        AGC_Basic_Linear(*thermal16, *thermal16_linear, height, width, camera_frame, camera_clock, FFC_mode);
        if ( this->VideoOutput==RAW16 ) {
            // push image to output queue
            img_frame_list[index]++;
            t_system = clock();
            img2queue = new cvMatContainer(*thermal16, img_frame_list[index], camera_clock, camera_frame, FFC_mode);
            (frame_queue[index])->push(img2queue);
        }else if(this->VideoOutput == RAW16_AGC)
        {
            img_frame_list[index]++;
            t_system = clock();
            img2queue = new cvMatContainer(*thermal16_linear, img_frame_list[index], camera_clock, camera_frame, FFC_mode);
            (frame_queue[index])->push(img2queue);
            /*
            //print fps
            int t_prev = last_frame_time_stamp[index];
            last_frame_time_stamp[index]=camera_clock;
            double fps = 1000000.0/(camera_clock-t_prev);
            cout<<"Camera "<<index<<" running at "<<fps<<" fps"<<endl;
            */

            /* 
            //show images debug
            cv::imshow(camera_ports[index], *thermal16_linear);
            char key = cv::waitKey(1);
            if( key == 'q' ) { // 0x20 (SPACE) ; need a small delay !! we use this to also add an exit option
                printf(WHT ">>> " RED "'q'" WHT " key pressed. Quitting !\n");
                break_out = true;
		    }
            */
        }else if(this->VideoOutput == AGC8)
        {
            cv::cvtColor(*thermal_luma, *thermal_RGB, cv::COLOR_YUV2RGB_I420, 0 );
            img_frame_list[index]++;
            t_system = clock();
            img2queue = new cvMatContainer(*thermal_RGB, img_frame_list[index], camera_clock, camera_frame, FFC_mode);
            (frame_queue[index])->push(img2queue);
        }
    }
    //tell the system that image buffer has stopped
    stream_stop_bool[index]=true;
}
 
void CameraStreamer::startMultiCapture(){

    // set buffer parameters  
    struct v4l2_format format; 
    if(this->VideoOutput==RAW16 || this->VideoOutput == RAW16_AGC)
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
        luma_height = height+height/2;
	    luma_width = width;
    }

    // Common varibles
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;

    struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 1; 

    struct v4l2_capability cur_cap;
    thread * t = NULL;
    tbb::concurrent_queue<cvMatContainer*>* q = NULL;
    int * cur_handel = NULL;
    struct v4l2_buffer* bufferinfo =NULL;
    
    for (int i = 0; i < camera_count; i++)
    {
        cur_handel = new int;
        *cur_handel = open((camera_ports[i]).c_str(), O_RDWR);
        //open device
        if((*cur_handel) < 0){
		    perror(RED "Error : OPEN. Invalid Video Device" WHT "\n");
		    exit(1);
	    } 

        // Check VideoCapture mode is available
        if(ioctl(*cur_handel, VIDIOC_QUERYCAP, &cur_cap) < 0){
            perror(RED "ERROR : VIDIOC_QUERYCAP. Video Capture is not available" WHT "\n");
            exit(1);
        }
        if(!(cur_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
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

        bufferinfo = (struct v4l2_buffer*)malloc(sizeof(struct v4l2_buffer));
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
        buffer_start_list.push_back(buffer_start);

        // Declarations for RAW16 representation
        // Will be used in case we are reading RAW16 format
	    // Boson320 , Boson 640
        cv::Mat* thermal16 = new cv::Mat(height, width, CV_16U, buffer_start);
        cv::Mat* thermal16_linear = new cv::Mat(height,width, CV_8U, 1);
        thermal16List.push_back(thermal16);
        thermal16linearList.push_back(thermal16_linear);

        // Declarations for 8bits YCbCr mode
        // Will be used in case we are reading YUV format
	    // Boson320, 640 :  4:2:0
        // OpenCV input buffer    
        cv::Mat* thermal_luma = new cv::Mat(luma_height, luma_width,  CV_8UC1, buffer_start);  
	    cv::Mat* thermal_rgb = new cv::Mat(height, width, CV_8UC3, 1);
        thermal_lumaList.push_back(thermal_luma);
        thermal_RGBList.push_back(thermal_rgb);

        // make default setting for index and 
        img_frame_list.push_back(0);
        stream_stop_bool.push_back(false);

        cur_handel = NULL;
        bufferinfo =NULL;
        
    }

    for(int i=0; i<camera_count; i++)
    {
        //Make thread instance
        t = new thread(&CameraStreamer::captureFrame, this, i);
        //Put thread to the vector
        camera_thread.push_back(t);
        //Make a queue instance
        q = new tbb::concurrent_queue<cvMatContainer*>;
        //Put queue to the vector
        frame_queue.push_back(q);
        t = NULL;
        q = NULL;
    }
}
 
bool CameraStreamer::stream_stopped()
{
    return this->break_out;
}

void CameraStreamer::requestQuit(bool input)
{
    this->break_out=input;
}

void CameraStreamer::stopMultiCapture()
{
    int *camera_handel_cur;
    struct v4l2_buffer * bufferinfo;
    for (int i = 0; i < camera_count; i++) {
        camera_handel_cur = camera_handel[i];
        bufferinfo = bufferPtrList[i];
        
        if( ioctl(*camera_handel_cur, VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0 ){
            perror(RED "VIDIOC_STREAMOFF" WHT);
            exit(1);
	    }
        while(!stream_stop_bool[i])
            continue;
        close(*camera_handel_cur);
        delete thermal16List[i];
        thermal16List[i]=NULL;
        delete camera_handel[i];
        camera_handel[i]=NULL;
        delete bufferPtrList[i];
        bufferPtrList[i]=NULL;
        printf(WHT ">>> Shut down the camera" YEL "%i" WHT "\n", i);
    }
}

// AGC Sample ONE: Linear from min to max.
// Input is a MATRIX (height x width) of 16bits. (OpenCV mat)
// Output is a MATRIX (height x width) of 8 bits (OpenCV mat)
void CameraStreamer::AGC_Basic_Linear(cv::Mat input_16, cv::Mat output_8, int height, 
    int width, unsigned int &frame, unsigned int &clock_cam, unsigned int &ffc_status) {
    int i, j;  // aux variables

    // auxiliary variables for AGC calcultion
    unsigned int max1=0;         // 16 bits
    unsigned int min1=0xFFFF;    // 16 bits
    unsigned int value1, value2, value3, value4;

    // RUN a super basic AGC
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
                        if(i==0 && j <=3)
                                continue;
            value1 =  input_16.at<uchar>(i,j*2+1) & 0XFF ;  // High Byte
            value2 =  input_16.at<uchar>(i,j*2) & 0xFF  ;    // Low Byte
            value3 = ( value1 << 8) + value2;
            if ( value3 <= min1 ) {
                min1 = value3;
            }
            if ( value3 >= max1 ) {
                max1 = value3;
            }
        }
    }
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
                        if(i==0 && j <= 3)
                                continue;
            value1 =  input_16.at<uchar>(i,j*2+1) & 0XFF ;  // High Byte
            value2 =  input_16.at<uchar>(i,j*2) & 0xFF  ;    // Low Byte
            value3 = ( value1 << 8) + value2;
            value4 = ( ( 255 * ( value3 - min1) ) ) / (max1-min1)   ;
            output_8.at<uchar>(i,j)= (uchar)(value4&0xFF);
        }
    }
        //unsigned int frame, clock_cam, ffc_status;
        frame = input_16.at<ushort>(0,0);
        clock_cam = (input_16.at<ushort>(0,2) << 16) + input_16.at<ushort>(0,1);
        ffc_status = input_16.at<ushort>(0,3);
}