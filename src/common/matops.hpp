#pragma once
#if !defined(__MATOPS_HPP_20111213205720__)
#define __MATOPS_HPP_20111213205720__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-13
 *  
 *  @section DESCRIPTION
 *  
 *  Matrix operations.
 */

#include <opencv2/opencv.hpp>

namespace cvcourse
{
    struct adl_t {};

    namespace
    {
        /// argument dependency lookup
        const adl_t adl;
    }

    bool all(const cv::Mat &m);

    bool any(const cv::Mat &m);
}

#endif // __MATOPS_HPP_20111213205720__
