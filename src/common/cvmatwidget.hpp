#if !defined(__CVMATWIDGET_HPP_2011112823046__)
#define __CVMATWIDGET_HPP_2011112823046__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-11-28
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QLabel>

#include <ans/pimpl.hpp>

namespace cv
{
    class Mat;
}

namespace cvcourse
{
    class cvmatwidget :
        public QLabel,
        public ans::pimpl<cvmatwidget>::pointer_semantics
    {
        Q_OBJECT

    public:
        cvmatwidget();

        cvmatwidget(cv::Mat mat);

    public:
        void set_mat(cv::Mat mat);

        cv::Mat mat() const;
    };
}

#endif // __CVMATWIDGET_HPP_2011112823046__
