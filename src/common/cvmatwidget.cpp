#include <boost/numeric/conversion/cast.hpp>

#include <opencv2/opencv.hpp>

#include <QFileDialog>
#include <QWheelEvent>

#include "cvmatwidget.hpp"
#include "cvmat2qimage.hpp"

template<>
struct ans::pimpl<cvcourse::cvmatwidget>::data
{
    cv::Mat mat;
    int scale;

    data() : scale(100) {}
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
        setPixmap(QPixmap::fromImage(cvmat2qimage(rgb)));
        resize(pixmap()->size());
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

void cvcourse::cvmatwidget::wheelEvent(QWheelEvent *e)
{
    auto scale = _data().scale += e->delta() / 120 * 5;
    if (auto p = pixmap())
    {
        cv::Mat rgb;
        cv::cvtColor(_data().mat, rgb, CV_BGR2RGB);
        cv::resize(rgb, rgb, cv::Size(), scale / 100.0, scale / 100.0);
        setPixmap(QPixmap::fromImage(cvmat2qimage(rgb)));
        resize(pixmap()->size());
    }
    base_type::wheelEvent(e);
}
