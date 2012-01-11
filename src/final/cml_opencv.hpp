#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __CML_OPENCV_HPP_20120110170053__
#define __CML_OPENCV_HPP_20120110170053__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-10
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <functional>

#include <boost/numeric/conversion/cast.hpp>

#include <cml/cml.h>

#include <opencv2/opencv.hpp>

namespace cml { namespace extension {

    template<class Target, class Source, class Enable = void>
    struct convert_impl;

    template<class Target, class Source>
    Target convert(Source source)
    {
        return convert_impl<Target, Source>()(source);
    }

    template<class T>
    struct convert_impl<vector<T, fixed<2> >, cv::Point> :
        std::unary_function<cv::Point, vector<T, fixed<2> > >
    {
        result_type operator()(argument_type v) const
        {
            return result_type(
                boost::numeric_cast<T>(v.x),
                boost::numeric_cast<T>(v.y)
                );
        }
    };

    template<class T>
    struct convert_impl<cv::Point, vector<T, fixed<2> > > :
        std::unary_function<vector<T, fixed<2> >, cv::Point>
    {
        result_type operator()(argument_type v) const
        {
            return result_type(
                boost::numeric_cast<int>(v[0]),
                boost::numeric_cast<int>(v[1])
                );
        }
    };
}
}

#endif // __CML_OPENCV_HPP_20120110170053__
