#include <cvMatContainer.hpp>

cvMatContainer::cvMatContainer(cv::Mat & input, int index, int time_stamp_camera, int time_stamp_buffer, int frame, int FFC_mode)
{
    this->img = input.clone();
    this->index = index;
    this->time_stamp_camera = time_stamp_camera;
    this->time_stamp_buffer = time_stamp_buffer;
    this->frame_camera = frame;
    this->compression_params.push_back(cv::IMWRITE_PXM_BINARY);
}

cvMatContainer::cvMatContainer(cv::Mat & input, int index, int time_stamp_camera, int time_stamp_buffer, int frame, int FFC_mode, std::vector<int> IMWRITE_PARAM)
{
    this->img = input.clone();
    this->index = index;
    this->time_stamp_camera = time_stamp_camera;
    this->time_stamp_buffer = time_stamp_buffer;
    this->frame_camera = frame;
    this->compression_params=IMWRITE_PARAM;
}
        
cvMatContainer::cvMatContainer(cv::Mat & input)
{
    this->img = input.clone();
    this->index = 0;
    this->time_stamp_camera = 0;
    this->time_stamp_buffer = 0;
    this->frame_camera = 0;
    this->compression_params.push_back(cv::IMWRITE_PXM_BINARY);
}

void cvMatContainer::assignIndex(int index)
{
    this->index = index;
}

int cvMatContainer::getIndex() const
{
    return this->index;
}

void cvMatContainer::assignTimeCam(int time_stamp)
{
    this->time_stamp_camera = time_stamp;
}

int cvMatContainer::getTimeCam() const
{
    return this->time_stamp_camera;
}

void cvMatContainer::assignTimeBuffer(int time_stamp)
{
    this->time_stamp_buffer = time_stamp;
}

int cvMatContainer::getTimeBuffer() const
{
    return this->time_stamp_buffer;
}

cv::Mat cvMatContainer::getImg() const
{
    return this->img;
}

int cvMatContainer::getFrameNum() const
{
    return this->frame_camera;
}

bool cvMatContainer::saveImg(string path) const
{
    return imwrite(path, this->img , this->compression_params );
}

int cvMatContainer::getFFCMode() const
{
    return this->FFC_mode;
}