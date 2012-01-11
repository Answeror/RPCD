/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-09
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <algorithm> // sort, lower_bound

#include <boost/assert.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <QDebug>

#include "candidate_radius.hpp"
#include "roundness.hpp"

namespace boda = boost::adaptors;

using namespace cvcourse;

namespace
{
    const double ROUNDNESS_THRESHOLD = 0.9;
    const double RADIUS_THRESHOLD = 5;

    radius_container remove_too_small_ones(
        const radius_container &radius,
        const radius_container standard_radius
        )
    {
        BOOST_ASSERT(!standard_radius.empty());

        return boost::copy_range<radius_container>(
            radius | boost::adaptors::filtered([&](double r){
                return r > RADIUS_THRESHOLD && r > standard_radius.front() / 2;
                })
            );
    }

    radius_container create_standard_radius_bins(const radius_container standard_radius)
    {
        BOOST_ASSERT(!standard_radius.empty());

        std::vector<double> bins;
        for (int i = 1, n = standard_radius.size(); i != n; ++i)
        {
            bins.push_back((standard_radius[i - 1] + standard_radius[i]) / 2.0);
        }
        return bins;
    }
}

std::vector<radius_container> cvcourse::candidate_radius(
    radius_container radius,
    const std::vector<double> &radios
    )
{
    BOOST_ASSERT(!radios.empty());
    //BOOST_ASSERT(!radius.empty());

    const int radius_kinds = radios.size();
    std::vector<radius_container> result(radius_kinds);

    // not too few
    if (!radius.empty())
    {
        // calculate standard radius
        const double largest_radius = *boost::max_element(radius);
        radius_container standard_radius;
        for each (auto r in radios) standard_radius.push_back(r * largest_radius);
        boost::sort(standard_radius);

        // remove small ones
        radius = remove_too_small_ones(radius, standard_radius);

        // not too few
        if (!radius.empty())
        {

#if 0
            {
                auto bins = create_standard_radius_bins(standard_radius);

                // accumulate
                for each (auto r in radius)
                {
                    int pos = boost::lower_bound(bins, r) - bins.begin();
                    result[pos].push_back(r);
                }
            }
#else
            /// kmeans
            {
                auto bins = create_standard_radius_bins(standard_radius);
                BOOST_ASSERT(bins.size() + 1 == radius_kinds);

                //cv::Mat1b labels(cv::Size(radius_kinds, 1), 0);
                //for (int i = 0, n = radius.size(); i != n; ++i)
                //{
                //    int kind = boost::lower_bound(bins, radius[i]) - bins.begin();
                //    BOOST_ASSERT(kind < radius_kinds);

                //    labels(i) = kind;
                //}
#if 0
                {
                    cv::Mat1i labels;

                    cv::Mat1f data(cv::Size(radius.size(), 1), 0);
                    for (int i = 0, n = radius.size(); i != n; ++i)
                    {
                        data(i) = radius[i];
                    }

                    /// no centers will cause crush!
                    cv::Mat centers(data.size(), data.type());
                    cv::TermCriteria termcrit(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0);
                    kmeans(data, radius_kinds, labels, termcrit, 3, cv::KMEANS_PP_CENTERS, centers);

                    for (auto i = labels.begin(); i != labels.end(); ++i)
                    {
                        result.at(*i).push_back(radius.at(i.pos().x));
                    }

                    qDebug() << QVector<double>::fromStdVector(result.front());
                }
#endif

                {
                    using namespace cv;
                    int clusterCount = radius_kinds;
                    int sampleCount = radius.size();
                    int k;

                    Mat points(sampleCount, 1, CV_32FC1), labels;
                    for (int i = 0, n = radius.size(); i != n; ++i)
                    {
                        points.at<float>(i) = radius[i];
                    }
                    //clusterCount = MIN(clusterCount, sampleCount);
                    Mat centers(clusterCount, 1, points.type());
                    kmeans(points, clusterCount, labels, 
                        TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0),
                        3, KMEANS_PP_CENTERS, centers);
                    std::vector<int> index;
                    for (int i = 0; i != radius_kinds; ++i) index.push_back(i);
                    boost::sort(index, [&](int lhs, int rhs){ return centers.at<float>(lhs) < centers.at<float>(rhs); });
                    for (auto i = labels.begin<int>(); i != labels.end<int>(); ++i)
                    {
                        result.at(index.at(*i)).push_back(radius.at(i - labels.begin<int>()));
                    }
                }
            }
#endif
        }
    }

    BOOST_ASSERT(
        boost::empty(result | boda::filtered([](const radius_container &rs){ return !rs.empty(); })) ||
        !result.back().empty()
        );

    return result;
}

bool cvcourse::candidate_contour(const contour &c)
{
    return roundness(c) >= ROUNDNESS_THRESHOLD && contour_radius(c) > RADIUS_THRESHOLD;
}

double cvcourse::contour_radius(const contour &c)
{
    auto rect = cv::boundingRect(c);
    return (rect.width + rect.height) / 4.0;
}

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace bacc = boost::accumulators;

cvcourse::radius_description::radius_description(const radius_container &radius)
{
    BOOST_ASSERT(!radius.empty());

    bacc::accumulator_set<double, bacc::stats<bacc::tag::mean, bacc::tag::variance> > acc;
    for each (auto r in radius) acc(r);

    this->mean = bacc::mean(acc);
    this->st = std::sqrt(bacc::variance(acc));
}

cvcourse::radius_description::radius_description(double mean, double st) : mean(mean), st(st)
{

}

double cvcourse::radius_description::best_guess() const
{
    return mean;
}

double cvcourse::radius_description::lower_bound() const
{
    return (std::max)(0.0, mean - (std::max)(3 * st, (std::max)(5.0, 0.1 * mean)));
}

double cvcourse::radius_description::upper_bound() const
{
    return mean + (std::max)(3 * st, (std::max)(5.0, 0.1 * mean));
}

cvcourse::radius_description cvcourse::radius_description::scale(double factor) const
{
    return radius_description(mean * factor, st * factor);
}
