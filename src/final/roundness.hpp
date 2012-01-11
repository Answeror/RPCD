#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __ROUNDNESS_HPP_20120109154015__
#define __ROUNDNESS_HPP_20120109154015__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-09
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <vector>

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    typedef std::vector<cv::Point> contour;

    /**
     *  Calculate contour roundness, 1 means round, others <1.
     */
    double roundness(const contour &c);
}

#endif // __ROUNDNESS_HPP_20120109154015__
