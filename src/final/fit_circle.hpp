#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __FIT_CIRCLE_HPP_2012011104908__
#define __FIT_CIRCLE_HPP_2012011104908__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-11
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <vector>

#include <cml/cml.h>

namespace cvcourse
{
    /// linear least-squares algorithm by Coope (1993) for circle fitting
    /// http://jsxgraph.uni-bayreuth.de/wiki/index.php/Least-squares_circle_fitting
    namespace fit_circle
    {
        typedef std::vector<cml::vector2d> point_container;

        struct result_type
        {
            cml::vector2d center;
            double radius;
            double mse; // mean square error
        };

        result_type go(const point_container &ps);
    }
}

#endif // __FIT_CIRCLE_HPP_2012011104908__
