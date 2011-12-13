#pragma once
#if !defined(__MAINWINDOW_HPP_2011121304845__)
#define __MAINWINDOW_HPP_2011121304845__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-13
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QMainWindow>

#include <ans/beta/pimpl.hpp>

namespace cvcourse
{
    class mainwindow : public QMainWindow
    {
        Q_OBJECT
            
    public:
        mainwindow();

    private slots:
        void open_input();
        void open_reference();
        void transfer();

    protected:
        struct data;
        struct method;
        typedef ans::beta::pimpl::use_default_ctor use_default_ctor;
        ans::beta::pimpl::unique<data, method> impl;
    };
};

#endif // __MAINWINDOW_HPP_2011121304845__
