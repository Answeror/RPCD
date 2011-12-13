#pragma once
#if !defined(__SATURATION_CAST_HPP_20111213211501__)
#define __SATURATION_CAST_HPP_20111213211501__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-13
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <limits>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/assert.hpp>

namespace cvcourse
{
    template<typename Target, typename Source>
    Target saturation_cast(Source src) {
        try {
            return boost::numeric_cast<Target>(src);
        }
        catch (const boost::numeric::negative_overflow &) {
            //BOOST_ASSERT(false);
            return std::numeric_limits<Target>::min();
        }
        catch (const boost::numeric::positive_overflow &) {
            //BOOST_ASSERT(false);
            return std::numeric_limits<Target>::max();
        }
    }
}

#endif // __SATURATION_CAST_HPP_20111213211501__
