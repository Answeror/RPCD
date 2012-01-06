#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __MAINWINDOW_HPP_20120106234635__
#define __MAINWINDOW_HPP_20120106234635__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QDialog>

#include <ans/alpha/pimpl.hpp>

namespace cvcourse
{
    class mainwindow : public QDialog
    {
        Q_OBJECT

    public:
        mainwindow();

    private Q_SLOTS:
        void hough();

    private:
        struct impl;
        ans::alpha::pimpl::unique<impl> self;
    };
}

#endif // __MAINWINDOW_HPP_20120106234635__
