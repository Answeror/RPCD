#include "algo.hpp"

cv::Mat3b cvcourse::canny(cv::Mat3b image, double low, double high, int aperture)
{
    cv::Mat3b gray;
    cv::cvtColor(image, gray, CV_BGR2GRAY);
    return gray;
}

