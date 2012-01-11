#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __ED_HPP_20120110151135__
#define __ED_HPP_20120110151135__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-10
 *  
 *  @section DESCRIPTION
 *  
 *  http://ceng.anadolu.edu.tr/cv/EDCircles/
 */

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    typedef std::vector<cv::Point> edge_segment;
    typedef std::vector<cv::Point> contour;

    edge_segment contour_to_edge_segment(const contour &c);

    typedef std::vector<edge_segment> edge_segment_container;
    typedef cv::Vec3f circle;
    typedef std::vector<circle> circle_container;

    circle_container edcircles(const edge_segment_container &es);
}

#endif // __ED_HPP_20120110151135__
