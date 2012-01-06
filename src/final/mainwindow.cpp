/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <opencv2/opencv.hpp>

#include <QPushButton>
#include <QHBoxLayout>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>

#include <boost/assert.hpp>

#include "mainwindow.hpp"
#include "yacvwindow.hpp"
#include "yacvwidget.hpp"

struct cvcourse::mainwindow::impl
{
    cvcourse::yacvwindow *rawwin;
};

namespace
{
    const ans::alpha::pimpl::use_default_ctor use_default_ctor;
}

cvcourse::mainwindow::mainwindow() : self(use_default_ctor)
{
    auto lay = new QHBoxLayout(this);
    setLayout(lay);

    auto hough = new QPushButton("hough", this);
    lay->addWidget(hough);
    connect(hough, SIGNAL(clicked()), this, SLOT(hough()));

    self->rawwin = new yacvwindow(this);
    self->rawwin->setWindowTitle("raw");
    self->rawwin->show();
}

//#include <iostream>

void cvcourse::mainwindow::hough()
{

    using namespace cv;

    BOOST_ASSERT(self->rawwin);
    cv::Mat src = self->rawwin->plot().get().clone();

    cv::Mat src_gray;

    //std::cout << src.empty() << "\n";

    /// Convert it to gray
    cvtColor( src, src_gray, CV_BGR2GRAY );

    //std::cout << "cvted\n";

    /// Reduce the noise so we avoid false circle detection
    GaussianBlur( src_gray, src_gray, Size(7, 7), 0);

    //std::cout << "blured\n";

    //cv::Mat canny;
    //Canny(src_gray, canny, 50, 100, 3);
    //src_gray = canny;

    //{
    //    cv::Mat result;
    //    cvtColor( src_gray, result, CV_GRAY2BGR );
    //    auto ca = new yacvwindow(this);
    //    ca->setWindowTitle("canny");
    //    ca->plot().set(result);
    //    ca->show();
    //}

    vector<Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.size().height / 3, 250, 100);

    /// Draw the circles detected
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        // circle center
        circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
        // circle outline
        circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }

    auto ya = new yacvwindow(this);
    ya->setWindowTitle("hough");
    cv::Mat result = src;
    ya->plot().set(result);
    ya->show();
}
