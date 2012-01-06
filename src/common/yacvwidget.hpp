#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __YACVWIDGET_HPP_20120106212348__
#define __YACVWIDGET_HPP_20120106212348__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  Yet another opencv widget.
 */

#define QWT_DLL

#include <qwt_plot.h>

#include <opencv2/opencv.hpp>

#include <ans/alpha/pimpl.hpp>

namespace cvcourse
{
    class yacvwidget : public QwtPlot
    {
        Q_OBJECT

    public:
        typedef yacvwidget this_type;
        typedef QwtPlot base_type;

    public:
        enum rescale_mode_type
        {
            NONE,
            FIXED,
            EXPANDING,
            FITTING
        };

    public:
        yacvwidget(QWidget *parent = nullptr);

    public Q_SLOTS:
        /// @return true if success
        bool load();
        void save();
        void set(const cv::Mat &mat);
        const cv::Mat& get() const;

        void set_rescale_mode(int mode);

    private:
        class impl;
        ans::alpha::pimpl::unique<impl> self;
    };
}

#endif // __YACVWIDGET_HPP_20120106212348__
