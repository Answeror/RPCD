#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __ALGO_HPP_20120108143848__
#define __ALGO_HPP_20120108143848__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-08
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <vector>

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    struct preprocess_params
    {
        int dilate_kernel_radio;
    };

    /**
     *  Do preprocess on input image.
     *  
     *  @param input BGR
     *  @return BGR
     */
    cv::Mat3b preprocess(const cv::Mat3b &input, const preprocess_params &params);


    struct detect_circles_params
    {
        double dp;
        double canny_high_threshold;
        double center_minimum_distance;
        double accumulator_threshold;
    };

    /**
     *  @param input BGR
     *  @return BGR
     */
    std::vector<cv::Vec3f> detect_disks(const cv::Mat3b &input, const detect_circles_params &params);

    /**
     *  Draw circles on image.
     *  
     *  @param input BGR
     *  @return BGR
     */
    cv::Mat3b draw_cycles(const cv::Mat3b &input, const std::vector<cv::Vec3f> &circles);
}

#endif // __ALGO_HPP_20120108143848__
