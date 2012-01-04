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

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    /**
     *  Canny edge detector.
     *  
     *  @param image BGR
     *  @param low low threshold
     *  @param high high threshold
     *  @param aperture kernel size
     *  @return GRAY
     */
    cv::Mat3b canny(cv::Mat3b image, double low, double high, int aperture);
}

#endif // __ALGO_HPP_20120104190157__
