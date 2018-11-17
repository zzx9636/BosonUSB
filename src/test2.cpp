#include <StereoSync.hpp>

int main()
{
    vector<string> capture_index = { "/dev/video1", "/dev/video2" };
    StereoSync sync = StereoSync(capture_index, Boson_640 ,RAW16_AGC, true, false);
    sync.startSync();
    return 0;
}