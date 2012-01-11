/**
*  @file
*  @author answeror <answeror@gmail.com>
*  @date 2012-01-08
*  
*  @section DESCRIPTION
*  
*  
*/

#include <cmath>
#include <algorithm>

#include <boost/assert.hpp>

#include "algo.hpp"
#include "yahc.hpp"

#include <qDebug>

using namespace cvcourse;

namespace
{
    cv::Mat1b show_contours(const cv::Mat1b &input)
    {
        cv::Mat1b dst(input.size(), 0);
        drawContours(dst, find_contours(input), -1, cv::Scalar(255), CV_FILLED);
        return dst;
    }

    std::vector<std::vector<cv::Point> > find_contours_from_original(const cv::Mat3b &input)
    {
        auto threshed = make_background_black(thresh(input));
        return find_contours(threshed);
    }
}

cv::Mat3b cvcourse::preprocess(const cv::Mat3b &input, const preprocess_params &params)
{
    auto threshed = make_background_black(thresh(input));

    threshed = show_contours(threshed);

    auto dilate_kernel_length = params.dilate_kernel_radio * 2 + 1;
    if (dilate_kernel_length > 1)
    {
        //cv::erode(threshed, threshed, cv::Mat1b(cv::Size(dilate_kernel_length, dilate_kernel_length), 255));
        //cv::dilate(threshed, threshed, cv::Mat1b(cv::Size(dilate_kernel_length, dilate_kernel_length), 255));
        auto elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(dilate_kernel_length, dilate_kernel_length));
        //cv::morphologyEx(threshed, threshed, cv::MORPH_OPEN, elem);
        //threshed = threshed ^ 255;
        cv::erode(threshed, threshed, elem);
    }

    //threshed = show_contours(threshed);

    cv::Mat3b result;
    cv::cvtColor(threshed, result, CV_GRAY2BGR);
    //cv::cvtColor(result - input, result, CV_GRAY2BGR);
    //result = result - input + input;

    return result;
}

namespace
{
    // white radio in one circle
    double white_radio(const cv::Vec3f &circle, const cv::Mat1b &image)
    {
        cv::Mat1b mask(image.size(), 0);
        cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        int radius = cvRound(circle[2]);
        // disk mask
        cv::circle(mask, center, radius, 255, 1, 8, 0 );
        auto nu = sum(image & mask)(0);
        auto de = sum(mask)(0);

        return de < 1e-8 ? 0 : nu / de;
    }

    double white_radio(const cv::Vec4f &circle)
    {
        //qDebug() << circle(0) << circle(1) << circle(2) << circle(3);
        auto r = cvRound(circle(2));
        auto d = cvRound(2 * circle(2));
        cv::Mat1b mask(cv::Size(d + 2, d + 2), 0);
        cv::circle(mask, cv::Point(r, r), r, 1, 1, 8, 0);

        static const double dpi = 2 * std::acos(-1.0);
        auto nu = circle(3);
        auto de = sum(mask)(0);
        return de < 1e-8 ? 0 : nu / de;
    }

    /// larger center in smaller circle
    bool collide(const cv::Vec3f &u, const cv::Vec3f &v)
    {
        auto dx = u(0) - v(0);
        auto dy = u(1) - v(1);
        auto d2 = dx * dx + dy * dy;
        return d2 < std::min(u(2) * u(2), v(2) * v(2));
    }
}

std::vector<cv::Vec3f> cvcourse::detect_disks(
    const cv::Mat3b &input, const detect_circles_params &params)
{
    BOOST_ASSERT(!input.empty());

    cv::Mat gray;

    cv::cvtColor(input, gray, CV_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);

    std::vector<cv::Vec3f> circles;

    /// just use hough circle
    {
        cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);
        cv::HoughCircles(
            gray,
            circles,
            CV_HOUGH_GRADIENT,
            2, //params.dp,
            params.maximum_radius / 2, //params.center_minimum_distance,
            100, //params.canny_high_threshold,
            50, //std::max(50.0, params.maximum_radius / 3), //params.accumulator_threshold,
            params.minimum_radius,
            params.maximum_radius
            );
    }

    /// use contour
#if 0
    {
        auto contours = find_contours_from_original(input);

        if (!contours.empty()) {

            int n = contours.size();

            qDebug() << "contours:" << n;

            std::vector<double> areas;
            for each (auto &contour in contours)
            {
                auto area = cv::contourArea(contour);
                areas.push_back(area);
            }

            double largest = *std::max_element(areas.begin(), areas.end());

            qDebug() << "largest:" << largest;

            for (int i = 0; i < n; ++i)
            {
                qDebug() << "area:" << areas[i];
                /// too small
                if (areas[i] * 9 < largest) continue;

                auto box = cv::boundingRect(contours[i]);
                auto part = gray(box);

                qDebug() << box.x << box.y << box.width << box.height;

                std::vector<cv::Vec3f> result;
                cv::HoughCircles(part, result, CV_HOUGH_GRADIENT,
                    params.dp,
                    params.center_minimum_distance,
                    params.canny_high_threshold,
                    params.accumulator_threshold,
                    0,
                    0);

                for each (auto &c in result)
                {
                    circles.push_back(cv::Vec3f(c(0) + box.x, c(1) + box.y, c(2)));
                }
            }
        }
    }
#endif

    //{
    //    auto candi = yahc(src_gray, 1, 30);
    //    for each (auto c in candi)
    //    {
    //          circles.push_back(cv::Vec3f(c(0), c(1), c(2)));
    //    }
    //}

    //{
    //    std::vector<Vec4f> candi;
    //    for each (auto c in yahc(src_gray, 1, 30))
    //    {
    //        candi.push_back(c);
    //    }
    //    int n = candi.size();

    //    std::vector<bool> bad(n, false);
    //    //std::vector<double> radio(n, 0);
    //    //for (int i = 0; i != n; ++i)
    //    //{
    //    //    radio[i] = white_radio(candi[i]);
    //    //    //qDebug() << radio[i];
    //    //    if (radio[i] < 0.5) bad[i] = true;
    //    //}

    //    for (int i = 0; i != n; ++i)
    //    {
    //        if (bad[i]) continue;
    //        for (int j = i + 1; j != n; ++j)
    //        {
    //            if (bad[j]) continue;
    //            if (collide(candi[i], candi[j]))
    //            {
    //                //qDebug() << i << " " << j << " " << candi[i](3) << " " << candi[j](3);
    //                auto ir = candi[i](2);
    //                auto ic = candi[i](3);
    //                auto jr = candi[j](2);
    //                auto jc = candi[j](3);
    //                //if (candi[j](3) * candi[i](2) > candi[i](3) * candi[j](2)) {
    //                if (jc * ir > ic * jr) {
    //                    bad[i] = true;
    //                } else {
    //                    bad[j] = true;
    //                }
    //            }
    //        }
    //    }

    //    for (int i = 0; i != n; ++i)
    //    {
    //        if (!bad[i]) circles.push_back(cv::Vec3f(candi[i](0), candi[i](1), candi[i](2)));
    //    }
    //}


    ///// Apply the Hough Transform to find the circles
    ////qDebug() << high;
    //HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT,
    //    params.dp,
    //    params.center_minimum_distance,
    //    params.canny_high_threshold,
    //    params.accumulator_threshold,
    //    0,
    //    0);

    //qDebug() << circles.size();

    //self->circles = circles;


    return circles;
}

cv::Mat3b cvcourse::draw_circles(const cv::Mat3b &input, const std::vector<cv::Vec3f> &circles)
{
    using namespace cv;

    cv::Mat3b src = input.clone();

    // Draw the circles detected
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        // circle center
        circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
        // circle outline
        circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }

    return src;
}

std::vector<std::vector<cv::Point> > cvcourse::find_contours(const cv::Mat1b &input)
{
    using namespace cv;

    auto src = input.clone();

    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;

    //findContours( src, contours, hierarchy,
    //    CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    findContours( src, contours, hierarchy,
        CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    //qDebug() << contours.size();

    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    //int idx = 0;
    //for( ; idx >= 0; idx = hierarchy[idx][0] )
    //{
    //    Scalar color( rand()&255, rand()&255, rand()&255 );
    //    //drawContours( dst, contours, idx, color, CV_FILLED, 8, hierarchy );
    //}


    return contours;
}

cv::Mat1b cvcourse::thresh(const cv::Mat3b &input)
{
#if 0
    cv::Mat gray;
    cv::cvtColor(input, gray, CV_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);
    cv::equalizeHist(gray, gray);
    cv::Mat otsu;
    //cv::threshold(gray, otsu, 1, 255, cv::THRESH_OTSU);
    cv::adaptiveThreshold(gray, otsu, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 3, 5);
    return otsu;
#endif

#if 1
    cv::Mat1b result;
    cv::cvtColor(input, result, CV_BGR2GRAY);
    cv::GaussianBlur(result, result, cv::Size(9, 9), 2, 2);
    cv::Canny(result, result, 10, 30, 3);
    auto elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, elem);
    //cv::dilate(result, result, elem);
    //qDebug() << "well";
    return result;
#endif

    //cv::Mat1b result;
    //cv::cvtColor(input, result, CV_BGR2GRAY);
    //auto elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    //cv::morphologyEx(result, result, cv::MORPH_GRADIENT, elem);
    //cv::threshold(result, result, 1, 255, cv::THRESH_OTSU);
    //return result;
}

cv::Mat1b cvcourse::make_background_black(const cv::Mat1b &input)
{
    //qDebug() << cv::mean(input)(0);
    if (cv::mean(input)(0) >= 128)
    {
        cv::Mat1b result;
        result = input ^ 255;
        return result;
    }
    return input;
}

cvcourse::circle_container cvcourse::merge_circles(const circle_container &circles)
{
    const int n = circles.size();
    std::vector<bool> bad(n, false);

    for (int i = 0; i != n; ++i)
    {
        if (bad[i]) continue;
        for (int j = i + 1; j != n; ++j)
        {
            if (bad[j]) continue;
            if (collide(circles[i], circles[j]))
            {
                // smaller remain
                if (circles[j](2) < circles[i](2)) {
                    bad[i] = true;
                } else {
                    bad[j] = true;
                }
            }
        }
    }

    circle_container result;
    for (int i = 0; i != n; ++i)
    {
        if (!bad[i]) result.push_back(circles[i]);
    }

    //qDebug() << result.size();

    //return result;
    return circles;
}
