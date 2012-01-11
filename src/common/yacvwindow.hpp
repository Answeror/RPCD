#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __YACVWINDOW_HPP_20120106235302__
#define __YACVWINDOW_HPP_20120106235302__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QMainWindow>
#include <QToolBar>

#include <ans/alpha/pimpl.hpp>

namespace cvcourse
{
    class yacvwidget;

    class yacvwindow : public QMainWindow
    {
        Q_OBJECT

    public:
        yacvwindow(QWidget*);

    public:
        void enable_load();
        void disable_load();

        const yacvwidget& plot() const;
        yacvwidget& plot();

    protected:
        struct impl;
        ans::alpha::pimpl::unique<impl> self;
    };

    /// for generate image
    class generated_yacvwindow : public yacvwindow
    {
        Q_OBJECT

    public:
        typedef generated_yacvwindow this_type;
        typedef yacvwindow base_type;

    public:
        generated_yacvwindow(QWidget *parent = 0);
    };

    class yacvtoolbar : public QToolBar
    {
        Q_OBJECT

    public:
        yacvtoolbar(QWidget*);

    public:
        QSize sizeHint() const override;
    };
}

#endif // __YACVWINDOW_HPP_20120106235302__
