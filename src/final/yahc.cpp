/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-08
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include "yahc.hpp"
#include "yahc/hough.hpp"

#include "hcd/hcd.h"

std::vector<cv::Vec4f> cvcourse::yahc(
    const cv::Mat1b &binary,
    double center_minimum_distance,
    double accumulator_threshold
    )
{
    //std::vector<cv::Vec4f> circles;
    //cv::yaHoughCircles( binary, circles, CV_HOUGH_GRADIENT, 1, center_minimum_distance, /*no use*/200, accumulator_threshold, 0, 0 );
    ////return circles;
    //
    //std::vector<cv::Vec4f> result = circles;
    //return result;
    // 

    HoughCircleDetector hcd;
    return hcd.yadetect(binary);
}
