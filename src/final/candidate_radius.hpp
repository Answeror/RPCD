#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __CANDIDATE_RADIUS_HPP_20120109155408__
#define __CANDIDATE_RADIUS_HPP_20120109155408__

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
    typedef std::vector<double> radius_container;

    /**
     *  Calculate actual radius from given contours and expect radius radios.
     *  
     *  The largest radius bin won't be empty, otherwise all bins empty.
     *  
     *  @param radios in (0, 1]. example: {0.5, 1} meas compare to largest 
     *      radius, the second large one is 0.5
     *  @return equal to radios count, some bins may be empty
     */
    std::vector<radius_container> candidate_radius(
        radius_container radius,
        const std::vector<double> &radios
        );

    struct radius_description
    {
        radius_description(const radius_container &radius);

        double best_guess() const;
        double lower_bound() const;
        double upper_bound() const;

        radius_description scale(double factor) const;

    private:
        radius_description(double mean, double st);

    private:
        double mean, st;
    };

    bool candidate_contour(const contour &c);

    double contour_radius(const contour &c);
}

#endif // __CANDIDATE_RADIUS_HPP_20120109155408__
