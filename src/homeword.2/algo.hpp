#pragma once
#if !defined(__ALGO_HPP_20111212230541__)
#define __ALGO_HPP_20111212230541__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-12
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    /**
     *  Color Transfer Based on Normalized Cumulative Hue Histograms
     *  
     *  @param in input image, HSV
     *  @param ref reference image, HSV
     *  @return HSV format
     */
    cv::Mat3b color_transfer(const cv::Mat3b &in, const cv::Mat3b &ref);

    /**
     *  Color Transfer between Images
     *  Erik Reinhard method
     *  
     *  @param in input image, RGB
     *  @param ref reference image, RGB
     *  @return RGB format
     */
    cv::Mat3b reinhard(const cv::Mat3b &in, const cv::Mat3b &ref);
}

#endif // __ALGO_HPP_20111212230541__
