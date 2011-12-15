#include <boost/numeric/conversion/cast.hpp>

#include <opencv2/opencv.hpp>

#include <QFileDialog>
#include <QWheelEvent>
#include <QResizeEvent>

#include "cvmatwidget.hpp"
#include "cvmat2qimage.hpp"

template<>
struct ans::pimpl<cvcourse::cvmatwidget>::data
{
    cv::Mat mat;
    QSize size;

    data() : size(0, 0) {}
};

template<>
struct ans::pimpl<cvcourse::cvmatwidget>::method
{
};


cvcourse::cvmatwidget::cvmatwidget() : impl(use_default_ctor())
{
}

cvcourse::cvmatwidget::cvmatwidget(cv::Mat mat) : impl(use_default_ctor())
{
    set_mat(mat);
}

void cvcourse::cvmatwidget::set_mat(cv::Mat mat)
{
    _data().mat = mat.clone();
    if (mat.empty()) {
        this->clear();
    } else {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, CV_BGR2RGB);
        auto p = QPixmap::fromImage(cvmat2qimage(rgb));
        _data().size = p.size();
        hide();
        show();
    }
}

cv::Mat cvcourse::cvmatwidget::mat() const
{
    return _data().mat;
}

bool cvcourse::cvmatwidget::load()
{
    QString name = QFileDialog::getOpenFileName(
        this,
        tr("Open image"),
        ".",
        tr("Image files (*.png *.jpg *jpeg *bmp *gif)")
        );

    bool ok = false;
    if (!name.isEmpty()) {
        set_mat(cv::imread(name.toAscii().data()));
        ok = !mat().empty();
    }
    return ok;
}

void cvcourse::cvmatwidget::resizeEvent(QResizeEvent *e)
{
    //_data().size = e->size();
    base_type::resizeEvent(e);
}

void cvcourse::cvmatwidget::paintEvent(QPaintEvent *e)
{
    if (!_data().mat.empty()) {
        cv::Mat rgb;
        cv::cvtColor(_data().mat, rgb, CV_BGR2RGB);
        cv::resize(rgb, rgb, cv::Size(_data().size.width(), _data().size.height()));
        setPixmap(QPixmap::fromImage(cvmat2qimage(rgb)));
    }
    base_type::paintEvent(e);
}

QSize cvcourse::cvmatwidget::sizeHint() const 
{
    return _data().size;
}
