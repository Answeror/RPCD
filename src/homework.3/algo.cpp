#include <cmath>
#include <stack>

#include <boost/assert.hpp>

#include "algo.hpp"

namespace
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
            auto val = mag(p);
            if (val > mag(u) && val >= mag(v)) *i = val;
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

    /// manually implementation
    cv::Mat1b mine(cv::Mat1b image, double low, double high, int aperture)
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
        //cv::Mat1b ht;
        //nms.convertTo(ht, ht.type());

        return ht;
    }
}

cv::Mat1b cvcourse::canny(cv::Mat1b image, double low, double high, int aperture)
{
    //return lib(image, low, high, aperture);
    return mine(image, low, high, aperture);
}

