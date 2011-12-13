#pragma once
#if !defined(__WHEEL_DISABLED_SCROLLAREA_HPP_2011121404342__)
#define __WHEEL_DISABLED_SCROLLAREA_HPP_2011121404342__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-14
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QScrollArea>

namespace cvcourse
{
    class wheel_disabled_scrollarea : public QScrollArea
    {
        Q_OBJECT

    private:
        typedef QScrollArea base_type;

    public:
        void wheelEvent(QWheelEvent *e) override;
    };
}

#endif // __WHEEL_DISABLED_SCROLLAREA_HPP_2011121404342__
