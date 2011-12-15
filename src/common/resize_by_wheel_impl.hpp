#pragma once
#if !defined(__RESIZE_BY_WHEEL_IMPL_HPP_20111215225227__)
#define __RESIZE_BY_WHEEL_IMPL_HPP_20111215225227__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-15
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QWidget>

namespace cvcourse
{
    class resize_by_wheel_impl : public QWidget
    {
        Q_OBJECT

    public:
        resize_by_wheel_impl(QWidget &child);

    public:
        /// zoom
        void wheelEvent(QWheelEvent *e) override;
        void paintEvent(QPaintEvent *e) override;

    private:
        int scale;
        QWidget &child;
    };
}

#endif // __RESIZE_BY_WHEEL_IMPL_HPP_20111215225227__
