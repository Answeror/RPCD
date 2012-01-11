/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-09
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <cmath>
#include <algorithm> // min, max

#include "roundness.hpp"

/// One way to tell if an object is an ellipse is to look at the relationship
/// of its area to its dimensions.  If its actual occupied area can be estimated
/// using the well-known area formula Area = PI*A*B, then it has a good chance of
/// being an ellipse.
/// from { http://goo.gl/RhVsx }
double cvcourse::roundness(const contour &c)
{
    auto actual_area = cv::contourArea(c);
    auto rect = cv::boundingRect(c);
    int A = rect.width / 2;
    int B = rect.height / 2;
    static double pi = std::acos(-1.0);
    double estimated_area = pi * A * B;

    if (estimated_area < 1e-8) return 0;

    double error = std::fabs(actual_area - estimated_area);
    return (std::max)(0.0, 1 - error / estimated_area) * (std::min)(A, B) / (std::max)(A, B);
}
