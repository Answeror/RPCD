#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __YAHC_HPP_20120108190650__
#define __YAHC_HPP_20120108190650__

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
    std::vector<cv::Vec4f> yahc(
        const cv::Mat1b &binary,
        double center_minimum_distance,
        double accumulator_threshold = 1
        );
}

#endif // __YAHC_HPP_20120108190650__
