#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __ALGO_HPP_20120104190157__
#define __ALGO_HPP_20120104190157__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-04
 *  
 *  @section DESCRIPTION
 *  
 *  Canny edge detector.
 */

#include <string>

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    /**
     *  Canny edge detector.
     *  
     *  @param image GRAY
     *  @param low low threshold
     *  @param high high threshold
     *  @param aperture kernel size
     *  @param impl select implementation
     *  @return GRAY
     */
    cv::Mat1b canny(cv::Mat1b image, double low, double high, int aperture, const std::string &impl);
}

#endif // __ALGO_HPP_20120104190157__
