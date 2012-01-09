#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __HOUGH_HPP_20120108192445__
#define __HOUGH_HPP_20120108192445__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-08
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <opencv2/opencv.hpp>

namespace cv
{
void yaHoughCircles( InputArray image, OutputArray circles,
                               int method, double dp, double minDist,
                               double param1=100, double param2=100,
                               int minRadius=0, int maxRadius=0 );
}

#endif // __HOUGH_HPP_20120108192445__
