#include <opencv2/opencv.hpp>

#include "cvmatwidget.hpp"
#include "cvmat2qimage.hpp"

template<>
struct ans::pimpl<cvcourse::cvmatwidget>::data
{
    cv::Mat mat;
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
    _data().mat = mat;
    if (mat.empty()) {
        this->clear();
    } else {
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        setPixmap(QPixmap::fromImage(cvmat2qimage(mat)));
        resize(pixmap()->size());
    }
}

cv::Mat cvcourse::cvmatwidget::mat() const
{
    return _data().mat;
}
