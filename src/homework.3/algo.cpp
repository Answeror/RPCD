#include <cmath>
#include <stack>

#include <boost/assert.hpp>

#include "algo.hpp"
#include "saturation_cast.hpp"

namespace cvcourse { namespace
{
    /// use cv::Canny
    cv::Mat1b lib(cv::Mat1b image, double low, double high, int aperture)
    {
        cv::Mat1b result;
        cv::Canny(image, result, low, high, aperture);
        return result;
    }

    /// can be accelerated by gpu
    void magnitude_and_orientation(
        cv::Mat1d &mag,
        cv::Mat1b &ori,
        const cv::Mat1d &dx,
        const cv::Mat1d &dy
        )
    {
        BOOST_ASSERT(dx.size() == dy.size());
        mag.create(dx.size());
        cv::magnitude(dx, dy, mag);
        //mag = cv::abs(dx) + cv::abs(dy);
        //cv::cartToPolar(dx, dy, mag, angle);
        ori.create(mag.size());
        for (auto i = ori.begin(); i != ori.end(); ++i)
        {
            auto sx = dx(i.pos());
            auto sy = dy(i.pos());
            if (std::abs(sx) < 1e-8) {
                *i = 2;
            } else {
                auto t = sy / sx;
                if (t > -0.4142 && t <= 0.4142) *i = 0;
                else if (t > 0.4142 && t <= 2.4142) *i = 1;
                else if (t > -2.4142 && t <= -0.4142) *i = 3;
                else t = 2;
            }
        }
    }

    cv::Mat1d non_maximum_suppression(const cv::Mat1d &mag, const cv::Mat1b &ori)
    {
        BOOST_ASSERT(mag.size() == ori.size());
        cv::Mat1d nms(mag.size(), 0);
        for (auto i = nms.begin(); i != nms.end(); ++i)
        {
            auto p = i.pos();

            // check edge
            if (
                p.x == 0 ||
                p.y == 0 ||
                p.x + 1 == nms.size().width ||
                p.y + 1 == nms.size().height
                ) continue;

            static const cv::Point offsets[] = {
                cv::Point(1, 0),
                cv::Point(1, 1),
                cv::Point(0, 1),
                cv::Point(-1, 1)
                };
            auto u = p + offsets[ori(p)];
            auto v = p - offsets[ori(p)];
            auto val = saturation_cast<int>(mag(p));
            if (val > saturation_cast<int>(mag(u)) && val >= saturation_cast<int>(mag(v))) *i = val;
        }
        return nms;
    }

    /// 8 connected
    cv::Mat1b hysteresis_threshold(const cv::Mat1d &nms, double low, double high)
    {
        BOOST_ASSERT(low <= high);
        cv::Mat1b ht(nms.size(), 255);

        std::stack<cv::Point> stk;
        // check if point valid
        for (auto i = ht.begin(); i != ht.end(); ++i)
        {
            auto p = i.pos();
            auto val = nms(p);
            if (nms(p) >= high) stk.push(p);
        }

        auto valid = [&](const cv::Point &p) {
            return p.x >= 0 && p.y >= 0 && p.x < ht.size().width && p.y < ht.size().height;
            };
        while (!stk.empty())
        {
            auto p = stk.top();
            stk.pop();
            if (0 == ht(p)) continue;
            ht(p) = 0;
            static const cv::Point offsets[] = {
                cv::Point(1, 0),
                cv::Point(1, 1),
                cv::Point(0, 1),
                cv::Point(-1, 1),
                cv::Point(-1, 0),
                cv::Point(-1, -1),
                cv::Point(0, -1),
                cv::Point(1, -1)
                };
            for each (auto &offset in offsets)
            {
                auto q = p + offset;
                if (valid(q) && nms(q) >= low) stk.push(q);
            }
        }

        return ht;
    }

    /// for debug
    cv::Mat1b to1b(const cv::Mat &m)
    {
        cv::Mat1b result;
        m.convertTo(result, result.type());
        return result;
    }

    /// manually implementation according to PPT
    cv::Mat1b ppt(cv::Mat1b image, double low, double high, int aperture)
    {
        //cv::GaussianBlur(image, image, cv::Size(aperture, aperture), 0);
        // step 1 and 2
        cv::Mat1d dx, dy;
        cv::Sobel(image, dx, dx.depth(), 1, 0, aperture);
        cv::Sobel(image, dy, dy.depth(), 0, 1, aperture);

        // step 3
        cv::Mat1d mag;
        cv::Mat1b ori;
        magnitude_and_orientation(mag, ori, dx, dy);

        // step 4
        auto nms = non_maximum_suppression(mag, ori);

        // step 5
        auto ht = hysteresis_threshold(nms, low, high);

        return ht;
    }

    /// yet another non maximum suppression, see:
    /// http://www.tomgibara.com/computer-vision/canny-edge-detector
    cv::Mat1d ya_non_maximum_suppression(
        const cv::Mat1d &mag,
        const cv::Mat1d &dx,
        const cv::Mat1d &dy
        )
    {
        BOOST_ASSERT(mag.size() == dx.size());
        BOOST_ASSERT(mag.size() == dy.size());

        cv::Mat1d nms(mag.size(), 0);
        for (int y = 1, yend = nms.size().height - 1; y < yend; ++y)
        {
            for (int x = 1, xend = nms.size().width - 1; x < xend; ++x)
            {
                auto xgrad = dx(y, x);
                auto ygrad = dy(y, x);
                auto gradmag = mag(y, x);

                auto nmag = mag(y - 1, x);
                auto smag = mag(y + 1, x);
                auto wmag = mag(y, x - 1);
                auto emag = mag(y, x + 1);
                auto nemag = mag(y - 1, x + 1);
                auto semag = mag(y + 1, x + 1);
                auto swmag = mag(y + 1, x - 1);
                auto nwmag = mag(y - 1, x - 1);

                decltype(nmag) t;
                if (
                    xgrad * ygrad <= 0
                        ? std::abs(xgrad) >= std::abs(ygrad)
                            ? (t = std::abs(xgrad * gradmag)) >= std::abs(ygrad * nemag - (xgrad + ygrad) * emag)
                                && t > std::abs(ygrad * swmag - (xgrad + ygrad) * wmag)
                            : (t = std::abs(ygrad * gradmag)) >= std::abs(xgrad * nemag - (ygrad + xgrad) * nmag)
                                && t > std::abs(xgrad * swmag - (ygrad + xgrad) * smag)
                        : std::abs(xgrad) >= std::abs(ygrad)
                            ? (t = std::abs(xgrad * gradmag)) >= std::abs(ygrad * semag + (xgrad - ygrad) * emag)
                                && t > std::abs(ygrad * nwmag + (xgrad - ygrad) * wmag)
                            : (t = std::abs(ygrad * gradmag)) >= std::abs(xgrad * semag + (ygrad - xgrad) * smag)
                                && t > std::abs(xgrad * nwmag + (ygrad - xgrad) * nmag)
                    ) {
                    nms(y, x) = gradmag;
                }
            }
        }
        return nms;
    }

    /// yet another implementation
    cv::Mat1b yaimpl(cv::Mat1b image, double low, double high, int aperture)
    {
        // step 1 and 2
        cv::Mat1d dx, dy;
        cv::Sobel(image, dx, dx.depth(), 1, 0, aperture);
        cv::Sobel(image, dy, dy.depth(), 0, 1, aperture);

        // step 3
        cv::Mat1d mag;
        cv::magnitude(dx, dy, mag);

        // step 4
        auto nms = ya_non_maximum_suppression(mag, dx, dy);

        // step 5
        auto ht = hysteresis_threshold(nms, low, high);

        return ht;
    }
}}

cv::Mat1b cvcourse::canny(cv::Mat1b image, double low, double high, int aperture, const std::string &impl)
{
    if ("mine" == impl) {
        return yaimpl(image, low, high, aperture);
    } else if ("opencv" == impl) {
        return lib(image, low, high, aperture);
    } else {
        BOOST_ASSERT(false);
        return image;
    }
}

