#if !defined(__MAINWINDOW_HPP_2011112804423__)
#define __MAINWINDOW_HPP_2011112804423__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-11-28
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QMainWindow>

#include <ans/pimpl.hpp>

namespace cvcourse
{
    class mainwindow :
        public QMainWindow,
        public ans::pimpl<mainwindow>::pointer_semantics
    {
        Q_OBJECT
            
    public:
        mainwindow();

    private slots:
        void open();
        void convert();
    };
};

#endif // __MAINWINDOW_HPP_2011112804423__
