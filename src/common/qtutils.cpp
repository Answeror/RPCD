#include <QScrollArea>

#include "qtutils.hpp"
#include "wheel_disabled_scrollarea.hpp"

std::unique_ptr<QScrollArea> cvcourse::scrolled(QWidget &widget, bool wheel_disabled)
{
    std::unique_ptr<QScrollArea> scroll(wheel_disabled ? new wheel_disabled_scrollarea : new QScrollArea);
    scroll->setBackgroundRole(QPalette::Dark);
    scroll->setWidget(&widget);
    return scroll;
}
