#include <QHBoxLayout>
#include <QWheelEvent>

#include "resize_by_wheel_impl.hpp"

cvcourse::resize_by_wheel_impl::resize_by_wheel_impl(QWidget &child) :
    scale(100),
    child(child)
{
    auto p = new QHBoxLayout;
    p->addWidget(&child);
    setLayout(p);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void cvcourse::resize_by_wheel_impl::wheelEvent(QWheelEvent *e)
{
    scale += e->delta() / 120 * 5;
    child.resize(child.size() * scale / 100.0);
    QWidget::wheelEvent(e);
}

void cvcourse::resize_by_wheel_impl::paintEvent(QPaintEvent *e)
{
    resize(child.sizeHint());
    QWidget::paintEvent(e);
}

