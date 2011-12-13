#include <cmath>

#include <boost/assert.hpp>

#include "algo.hpp"
#include "matops.hpp"
#include "saturation_cast.hpp"

namespace cvcourse
{
    namespace
    {
        cv::MatND f(const cv::Mat3b &hsv)
        {
            int histsize[] = {180};
            float hranges[] = {0, 180};
            const float *ranges[] = {hranges};
            int channels[] = {0, 1};
            cv::MatND h;
            cv::calcHist(
                &hsv,
                1,
                channels,
                cv::Mat(),
                h,
                1,
                histsize,
                ranges,
                true,
                false
                );
            return h;
        }

        cv::MatND fnorm(const cv::Mat3b &image)
        {
            auto hist = f(image);
            cv::normalize(hist, hist, 1, cv::NORM_L1);
            return hist;
        }

        cv::MatND fcom(const cv::Mat3b &image)
        {
            auto h = fnorm(image);
            for (int i = 1; i < 180; ++i)
            {
                h.at<float>(i) += h.at<float>(i - 1);
            }
            return h;
        }

        unsigned char argmin(
            cv::Point k,
            const cv::MatND &fincom,
            const cv::MatND &frefcom,
            const cv::Mat3b &hin
            )
        {
            float rhs = fincom.at<float>(hin.at<cv::Vec3b>(k)[0]);
            int minidx = 0;
            float minval = 1;
            for (int i = 0; i < 180; ++i)
            {
                float val = std::abs(frefcom.at<float>(i) - rhs);
                if (val < minval)
                {
                    minval = val;
                    minidx = i;
                }
            }
            return saturation_cast<unsigned char>(minidx);
        }
    }
}

cv::Mat3b cvcourse::color_transfer(const cv::Mat3b &in, const cv::Mat3b &ref)
{
    auto fincom = fcom(in);
    auto frefcom = fcom(ref);
    cv::Mat3b out;
    out = in.clone();
    for (auto i = out.begin(); i != out.end(); ++i)
    {
        (*i)[0] = argmin(i.pos(), fincom, frefcom, in);
    }
    return out;
}

namespace cvcourse
{
    namespace
    {
        /// convert elemwise
        template<class To, class From, class Op>
        To convert(const From &rgb, Op op) 
        {
            To lms(rgb.size());
            for (auto i = std::begin(rgb); i != std::end(rgb); ++i)
            {
                lms.at<typename To::value_type>(i.pos()) =
                    op(rgb.at<typename From::value_type>(i.pos()));
            }
            return lms;
        }

        /// mult 3x3 matrix and 3 vector
        template<class V, class T, class U>
        V mul3(const T &t, const U &u)
        {
            typename cv::Vec<T::value_type, 3> a(u(0), u(1), u(2));
            auto w = t * a;
            typedef typename V::value_type value_type;
            return V(
                saturation_cast<value_type>(w(0)),
                saturation_cast<value_type>(w(1)),
                saturation_cast<value_type>(w(2))
                );
        }

        cv::Vec3f rgb2lms(const cv::Vec3b &rgb)
        {
            static const cv::Matx33f m(
                0.3811f, 0.5783f, 0.0402f,
                0.1967f, 0.7244f, 0.0782f,
                0.0241f, 0.1288f, 0.8444f
                );
            return mul3<cv::Vec3f>(m, rgb);
            //return convert<cv::Mat3f>(rgb,
            //    [&](const cv::Vec3b &u){ return mul3<cv::Vec3f>(m, u); });
        }

        cv::Vec3b lms2rgb(const cv::Vec3f &lms)
        {
            static const cv::Matx33f m(
                4.4679f, -3.5873f, 0.1193f,
                -1.2186f, 2.3809f, -0.1624f,
                0.0497f, -0.2439f, 1.2045f
                );
            return mul3<cv::Vec3b>(m, lms);
            //return convert<cv::Mat3b>(lms,
            //    [&](const cv::Vec3f &u){ return mul3<cv::Vec3b>(m, u); });
        }

        cv::Vec3f lms2LMS(const cv::Vec3f &lms)
        {
            return cv::Vec3f(std::log(lms(0)), std::log(lms(1)), std::log(lms(2)));
            //return convert<cv::Mat3f>(lms,
            //    [&](const cv::Vec3f &u){
            //        return cv::Vec3f(std::log(u(0)), std::log(u(1)), std::log(u(2)));
            //        });
        }

        cv::Vec3f LMS2lms(const cv::Vec3f &LMS)
        {
            return cv::Vec3f(std::exp(LMS(0)), std::exp(LMS(1)), std::exp(LMS(2)));
            //return convert<cv::Mat3f>(LMS,
            //    [&](const cv::Vec3f &u){
            //        return cv::Vec3f(std::exp(u(0)), std::exp(u(1)), std::exp(u(2)));
            //        });
        }

        cv::Vec3f LMS2lab(const cv::Vec3f &LMS)
        {
            static const cv::Matx33f m =
                cv::Matx33f(
                    1 / std::sqrt(3.0f), 0, 0,
                    0, 1 / std::sqrt(6.0f), 0,
                    0, 0, 1 / std::sqrt(2.0f)
                    ) *
                cv::Matx33f(
                    1, 1, 1,
                    1, 1, -2,
                    1, -1, 0
                    );
            return mul3<cv::Vec3f>(m, LMS);
            //return convert<cv::Mat3f>(LMS,
            //    [&](const cv::Vec3f &u){ return mul3<cv::Vec3f>(m, u); });
        }

        cv::Vec3f lab2LMS(const cv::Vec3f &lab)
        {
            static const cv::Matx33f m =
                cv::Matx33f(
                    1, 1, 1,
                    1, 1, -1,
                    1, -2, 0
                    ) *
                cv::Matx33f(
                    std::sqrt(3.0f) / 3, 0, 0,
                    0, std::sqrt(6.0f) / 6, 0,
                    0, 0, std::sqrt(2.0f) / 2
                    );
            return mul3<cv::Vec3f>(m, lab);
            //return convert<cv::Mat3f>(lab,
            //    [&](const cv::Vec3f &u){ return mul3<cv::Vec3f>(m, u); });
        }

        cv::Vec3f rgb2lab(const cv::Vec3b &rgb)
        {
            return LMS2lab(lms2LMS(rgb2lms(rgb)));
        }

        cv::Vec3b lab2rgb(const cv::Vec3f &lab)
        {
            return lms2rgb(LMS2lms(lab2LMS(lab)));
        }

        cv::Mat3f rgb2lab(const cv::Mat3b &rgb)
        {
            return convert<cv::Mat3f>(rgb, [&](const cv::Vec3b &u){ return LMS2lab(lms2LMS(rgb2lms(u))); });
        }

        cv::Mat3b lab2rgb(const cv::Mat3f &lab)
        {
            return convert<cv::Mat3b>(lab, [&](const cv::Vec3f &u) { return lms2rgb(LMS2lms(lab2LMS(u))); });
        }

        cv::Mat1f deal_one_channel(const cv::Mat1f &s, const cv::Mat1f &t)
        {
            cv::Scalar means, stddevs, meant, stddevt;
            cv::meanStdDev(s, means, stddevs);
            cv::meanStdDev(t, meant, stddevt);
            return meant[0] + (stddevt / stddevs)[0] * (s - means[0]);
        }
    }
}

cv::Mat3b cvcourse::reinhard(const cv::Mat3b &in, const cv::Mat3b &ref)
{
    std::vector<cv::Mat3f> incs;
    cv::split(rgb2lab(in), incs);
    std::vector<cv::Mat3f> refcs;
    cv::split(rgb2lab(ref), refcs);
    std::vector<cv::Mat> outcs;
    for (int i = 0; i < 3; ++i)
    {
        outcs.push_back(deal_one_channel(incs.at(i), refcs.at(i)));
    }
    cv::Mat3f out;
    cv::merge(outcs, out);
    return lab2rgb(out);
}