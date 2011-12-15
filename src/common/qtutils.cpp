#include <QScrollArea>

#include "qtutils.hpp"
#include "wheel_disabled_scrollarea.hpp"
#include "resize_by_wheel_impl.hpp"

std::unique_ptr<QScrollArea> cvcourse::scrolled(QWidget &widget, bool wheel_disabled)
{
    std::unique_ptr<QScrollArea> scroll(wheel_disabled ? new wheel_disabled_scrollarea : new QScrollArea);
    scroll->setBackgroundRole(QPalette::Dark);
    scroll->setWidget(&widget);
    return scroll;
}

std::unique_ptr<QWidget> cvcourse::resize_by_wheel(QWidget &widget)
{
    return std::unique_ptr<QWidget>(new resize_by_wheel_impl(widget));
}
