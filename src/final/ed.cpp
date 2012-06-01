/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-10
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <cmath>
#include <algorithm>

#include <boost/assign.hpp>
#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/as_container.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm/reverse.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/fill.hpp>
#include <boost/range/access.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/max_element.hpp>
//#include <boost/range/algorithm/rotate.hpp>

namespace br = boost::range;
namespace bra = br::access;

#include <boost/numeric/interval.hpp>
#include <boost/numeric/interval/io.hpp>
#include <boost/numeric/interval/compare/lexicographic.hpp>
namespace bn = boost::numeric;
using namespace bn::interval_lib::compare::lexicographic;

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>

#include <boost/numeric/ublas/lu.hpp>

#include <QDebug>

#include "ed.hpp"
#include "cml_opencv.hpp"
//#include "yacvwindow.hpp"
//#include "yacvwidget.hpp"
#include "fit_circle.hpp"
//#include "cml/mathlib/vector_angle.h"

namespace ba = boost::assign;
namespace cmle = cml::extension;
namespace bada = boost::adaptors;
namespace bacc = boost::accumulators;
using namespace cvcourse;

namespace
{
    static const double pi = std::acos(-1.0);
    static const double COVER_THRESHOLD = 0.5;
    static const double COMBINE_THRESHOLD = 5;
    static const double COMBINE_THRESHOLD_RELATIVE = 0.3;
    static const double CONTOUR_APPROXIMATION_THRESHOLD = 2;
    static const double TURN_LOWER_THRESHOLD = 6;
    static const double TURN_UPPER_THRESHOLD = 60;
    static const double FIT_CIRCLE_THRESHOLD = 0.9;
    static const double EDGE_LENGTH_THRESHOLD = 10;
    static const double MSE_THRESHOLD = 5;
    // TODO duplicated in mainwindow.cpp
    const double IDEAL_RADIUS = 50;
    const double P2E_THRESHOLD = std::sin(cml::rad(60.0));

    /// accessors
    float x(const circle &c) { return c(0); }
    float y(const circle &c) { return c(1); }
    float r(const circle &c) { return c(2); }
    float& x(circle &c) { return c(0); }
    float& y(circle &c) { return c(1); }
    float& r(circle &c) { return c(2); }

    bool good_mse(double mse, double radius)
    {
        return mse < std::max(MSE_THRESHOLD, radius * MSE_THRESHOLD / IDEAL_RADIUS);
    }
    
    struct turn
    {
        double radian;
        bool positive;
        bool good;

        turn() {}
        turn(const cv::Point &u, const cv::Point &v)
        {
            //BOOST_ASSERT(u != v);
            auto ulen = std::sqrt(u.ddot(u));
            BOOST_ASSERT(ulen > 1e-8);
            auto vlen = std::sqrt(v.ddot(v));
            BOOST_ASSERT(vlen > 1e-8);
            radian = std::acos(u.ddot(v) / (ulen * vlen));
            positive = (u.x * v.y - u.y * v.x >= 0);

            /// wrong
            //auto longer = std::max(length(cmle::convert<cml::vector2d>(u)), length(cmle::convert<cml::vector2d>(v)));
            //auto f = [&](double theta){ return theta * longer / (IDEAL_RADIUS * std::sin(theta / 2)); };
            //qDebug() << f(cml::rad(TURN_LOWER_THRESHOLD)) << f(cml::rad(TURN_UPPER_THRESHOLD));
            //good = radian >= f(cml::rad(TURN_LOWER_THRESHOLD)) && radian <= f(cml::rad(TURN_UPPER_THRESHOLD));

            good = cml::deg(radian) >= TURN_LOWER_THRESHOLD && cml::deg(radian) <= TURN_UPPER_THRESHOLD;
        }

        bool valid() const
        {
            //return cml::deg(radian) >= TURN_LOWER_THRESHOLD && cml::deg(radian) <= TURN_UPPER_THRESHOLD;
            return good;
        }
    };
    typedef std::vector<turn> turn_container;

    turn_container calculate_turns(const edge_segment &e)
    {
        BOOST_ASSERT(e.size() > 2);

        std::vector<cv::Point> ds;
        for (int i = 1, n = e.size(); i != n; ++i)
        {
            BOOST_ASSERT(e[i] != e[i - 1]);
            ds.push_back(e[i] - e[i - 1]);
        }
        ds.push_back(ds.front());

        turn_container turns;
        for (int i = 1, n = ds.size(); i != n; ++i)
        {
            turns.push_back(turn(ds[i - 1], ds[i]));
        }
        BOOST_ASSERT(turns.size() + 1 == e.size());

        return turns;
    }

    struct arc;
    typedef boost::optional<arc> maybe_arc;
    const auto none = boost::none_t();

    /// [0, 2pi]
    double angle_from_to(cml::vector2d u, cml::vector2d v)
    {
        return std::fmod(2 * pi + signed_angle_2D(u, v), 2 * pi);
    }

    bool same(cml::vector2d u, cml::vector2d v)
    {
        return length_squared(u - v) < 1e-8;
    }

    /// 3 point cover on circle
    double cover3(cml::vector2d u, cml::vector2d v, cml::vector2d w)
    {
        //double result = std::fabs(angle_from_to(u, v) + angle_from_to(v, w));
        //if (result <= 2 * pi) return result;
        //return std::fabs(angle_from_to(w, v) + angle_from_to(v, u));

        BOOST_ASSERT(!same(u, v));
        BOOST_ASSERT(!same(v, w));
        //BOOST_ASSERT(u != w);
        if (same(u, w)) return 2 * pi;
        double alpha = cml::unsigned_angle_2D(u, w);
        if (cml::perp_dot(w - u, v - u) * cml::perp_dot(w - u, -u) > 0) alpha = 2 * pi - alpha;
        return alpha;
    }

    template<class PointRange>
    bn::interval<double> calculate_cover(cml::vector2d center, const PointRange &points)
    {
        /// old code, wrong when > 2 pi
#if 0
        BOOST_ASSERT(boost::size(points) > 2);

        // find reference point
        auto mid = boost::find_if(points, [&](cml::vector2d p){
            return !same(p, points.front()) && !same(p, points.back());
        });
        BOOST_ASSERT(mid != points.end());

        // calculate radian covered
        return cover3(
            points.front() - center,
            *mid - center,
            points.back() - center
            );
#endif

        BOOST_ASSERT(boost::size(points) > 2);
        
        auto angle = boost::accumulate(
            boost::irange<int>(1, boost::size(points))
            | bada::transformed([&](int i){
                return signed_angle_2D(bra::at(points, i - 1) - center, bra::at(points, i) - center);
                }),
            0.0
            );
        auto init = signed_angle_2D(cml::vector2d(1, 0), bra::front(points) - center);
        auto fini = init + angle;
        if (fini < init) std::swap(init, fini);
        return bn::interval<double>(init, fini);
    }

    typedef std::vector<bn::interval<double> > interval_container;

    interval_container merge(interval_container intervals)
    {
        interval_container result;
        if (boost::empty(intervals)) return result;

        boost::sort(intervals, [&](bn::interval<double> lhs, bn::interval<double> rhs){
            return lower(lhs) < lower(rhs) || lower(lhs) == lower(rhs) && upper(lhs) < upper(rhs);
            });
        result.push_back(bra::front(intervals));
        std::for_each(++boost::begin(intervals), boost::end(intervals), [&](bn::interval<double> i){
            if (overlap(bra::back(result), i)) {
                bra::back(result) = hull(bra::back(result), i);
            } else {
                result.push_back(i);
            }
            });

        return result;
    }

    interval_container to_std_angle(bn::interval<double> i)
    {
        if (width(i) >= 2 * pi) return interval_container(1, bn::interval<double>(0, 2 * pi));
        auto a = fmod(fmod(lower(i), 2 * pi) + 2 * pi, 2 * pi);
        auto b = a + width(i);
        bn::interval<double> r(a, 2 * pi);
        if (b <= 2 * pi) return interval_container(1, bn::interval<double>(a, b));
        bn::interval<double> s(0, fmod(b, 2 * pi));
        if (upper(s) >= a) return interval_container(1, bn::interval<double>(0, 2 * pi));
        return ba::list_of(s)(r);
    }

    double covered(const interval_container &intervals)
    {
        interval_container ins;
        for each (auto i in intervals) ba::push_back(ins).range(to_std_angle(i));
        //qDebug() << "b4merge";
        //for each (auto i in ins) qDebug() << lower(i) << upper(i);
        //qDebug() << "b4merge";
        ins = merge(ins);
        //qDebug() << "aftmerge";
        //for each (auto i in ins) qDebug() << lower(i) << upper(i);
        //qDebug() << "aftmerge";
        return boost::accumulate(ins | bada::transformed([&](bn::interval<double> i){ return width(i); }), 0.0);
    }

    template<class PointContainerRange>
    fit_circle::result_type bag(const PointContainerRange &pcs, double target_cover)
    {
        BOOST_ASSERT(!boost::empty(pcs));

        auto front = boost::range::access::front;
        auto at = boost::range::access::at;

        const int n = boost::size(pcs);

        if (n == 1) return fit_circle::go(front(pcs));

        // mse for all points
        fit_circle::point_container ps;
        for each (auto &pc in pcs) ba::push_back(ps).range(pc);
        auto gret = fit_circle::go(ps);

        std::vector<double> cost(n, 0);
        // init cost
        for (int i = 0; i != n; ++i)
        {
            fit_circle::point_container ps;
            for (int j = 0; j != n; ++j)
            {
                if (i == j) continue;
                ba::push_back(ps).range(at(pcs, j));
                cost[i] = std::max(0.0, gret.mse - fit_circle::go(ps).mse);
            }
        }

        std::vector<double> benefit(n, 0);
        interval_container intervals = pcs |
            bada::transformed([&](decltype(bra::front(pcs)) ps){ return calculate_cover(gret.center, ps); }) |
            boost::as_container;
        auto gcover = covered(intervals);
        // init benefit
        for (int i = 0; i != n; ++i)
        {
            auto raw = covered(boost::irange<int>(0, n) |
                bada::filtered([&](int j){ return i != j; }) |
                bada::transformed([&](int i){ return intervals[i]; }) |
                boost::as_container
                );
            benefit[i] = gcover - raw;
            BOOST_ASSERT(benefit[i] > -1e-8);
            //benefit[i] = 1;
        }

        const double inf = 1e8;

        // init radios
        std::vector<double> radios(n, 0);
        for (int i = 0; i != n; ++i)
        {
            radios[i] = cost[i] < 1e-8 ? inf : benefit[i] / cost[i];
        }
        std::vector<int> indices = boost::irange<int>(0, n) | boost::as_container;
        boost::sort(indices, [&](int lhs, int rhs){ return radios[lhs] > radios[rhs]; });

        double total_cover = 0;
        int current_index = 0;
        while (current_index != n && total_cover < target_cover + 1e-8)
        {
            total_cover += benefit[indices[current_index]];
            ++current_index;
        }

        {
            fit_circle::point_container ps;
            for each (auto i in boost::irange<int>(0, current_index))
            {
                ba::push_back(ps).range(at(pcs, indices[i]));
            }
            return fit_circle::go(ps);
        }
    }

    struct arc
    {
        //edge_segment_container points;
        typedef std::vector<cml::vector2d> polygon;
        polygon points;
        circle guess;
        //double cover;
        //std::vector<bn::interval<double> > intervals;
        bn::interval<double> interval;

        arc() {}

        static maybe_arc make(const polygon &points)
        {

            arc a;

#if 0
            {
                if (points.size() < 7) return none;
                auto rect = cv::fitEllipse(points);

                if (std::min(rect.size.width, rect.size.height) / std::max(rect.size.width, rect.size.height) < FIT_CIRCLE_THRESHOLD) return none;

                x(a.guess) = rect.center.x;
                y(a.guess) = rect.center.y;
                r(a.guess) = (rect.size.width + rect.size.height) / 4.0;
                /// wrong
                a.cover = cover3(
                    cmle::convert<cml::vector2d>(points[0]),
                    cmle::convert<cml::vector2d>(points[1]),
                    cmle::convert<cml::vector2d>(points.back())
                    );
            }
#endif
            {
                if (points.size() < 3) return none;

                auto ret = fit_circle::go(points);

                //if (ret.mse > MSE_THRESHOLD) return none;
                if (!good_mse(ret.mse, ret.radius)) return none;

                /// check if edge near center
                {
                    auto p2e = [](cml::vector2d p, cml::vector2d a, cml::vector2d b)->double
                    {
                        BOOST_ASSERT(!same(a, b));
                        return std::fabs(perp_dot(p - b, a - b)) / length(a - b);
                    };
                    for each (auto i in boost::irange<int>(1, boost::size(points)))
                    {
                        if (p2e(ret.center, points[i - 1], points[i]) < ret.radius * P2E_THRESHOLD) return none;
                    }
                }

                a.points = points;
                x(a.guess) = ret.center[0];
                y(a.guess) = ret.center[1];
                r(a.guess) = ret.radius;
                a.interval = calculate_cover(ret.center, points);

                //double scale = 1;
                //if (length(ret.center - cml::vector2d(55, 50) * scale) < 10 * scale)
                //{
                //    qDebug() << "---";
                //    qDebug() << "points:" << points.size();
                //    for each (auto p in points)
                //    {
                //        qDebug() << p[0] << p[1];
                //    }
                //    qDebug() << "center:" << ret.center[0] << ret.center[1];
                //    qDebug() << "radius:" << ret.radius;
                //    qDebug() << "mse" << ret.mse;
                //    qDebug() << "cover:" << covered(interval_container(1, a.interval)) / (2 * pi);
                //    qDebug() << "interval width:" << width(a.interval) / (2 * pi);
                //    qDebug() << "interval:" << lower(a.interval) << upper(a.interval);
                //}
            }
            //qDebug() << points.size();
            //for each (auto p in points)
            //{
            //    qDebug() << p.x << p.y;
            //}
            //qDebug() << a.cover;

            //cv::Mat3b m(500, 500, 0);
            //std::vector<edge_segment> es(1, points);
            //cv::polylines(m, es, false, cv::Scalar(255));
            //auto ya = new generated_yacvwindow();
            //ya->plot().set(m);
            //ya->show();

            //qDebug() << cover3(cml::vector2d(1, 0), cml::vector2d(0, 1), cml::vector2d(-1, 0));
            //qDebug() << cover3(cml::vector2d(-1, 0), cml::vector2d(0, 1), cml::vector2d(1, 0));
            //qDebug() << cover3(cml::vector2d(1, 0), cml::vector2d(0, 1), cml::vector2d(0, -1));
            
            return a;
        }
    };
    typedef std::vector<arc> arc_container;

    arc_container calculate_arcs(const edge_segment &e)
    {
        //BOOST_ASSERT(e.size() > 2);
        if (e.size() <= 2) return arc_container();

        auto turns = calculate_turns(e);
        //BOOST_ASSERT(!turns.empty());

        if (turns.empty()) return arc_container();

        /// calculate median, to break on too long edge
        bacc::accumulator_set<double, bacc::stats<bacc::tag::median> > acc;
        std::vector<double> lens;
        auto elen = [&](int i){ return length(cmle::convert<cml::vector2d>(e[i + 1] - e[i])); };
        for (int i = 1, n = e.size(); i != n; ++i)
        {
            lens.push_back(elen(i - 1));
            acc(lens.back());
        }
        auto median = bacc::median(acc);
        //auto bade = [&](int i){ return elen(i) > EDGE_LENGTH_THRESHOLD * median; };
        auto bade = [&](int i){ return false; };

        int current_turn_index = 0;
        const int turn_count = turns.size();
        arc_container result;

        while (current_turn_index != turn_count)
        {
            // find first valid turn
            while (
                current_turn_index != turn_count &&
                (!turns[current_turn_index].valid() || bade(current_turn_index) || bade(current_turn_index + 1))
                ) ++current_turn_index;
            
            if (current_turn_index == turn_count) break;

            const int same_direction_begin = current_turn_index;
            const bool positive = turns[current_turn_index].positive;
            auto make = [&]
            {
                return arc::make(
                    e | bada::sliced(same_direction_begin, current_turn_index + 2) |
                    bada::transformed(&cmle::convert<cml::vector2d, cv::Point>) |
                    boost::as_container
                    );
            };
            auto try_push = [&]
            {
                //qDebug() << "try push" << same_direction_begin << current_turn_index;
                if (auto ret = make()) {
                    result.push_back(*ret);
                    //qDebug() << "pushed";
                } else {
                    --current_turn_index;
                    if (auto ret = make())
                    {
                        result.push_back(*ret);
                    }
                    ++current_turn_index;
                }
            };

            while (current_turn_index != turn_count)
            {
                auto &t = turns[current_turn_index];

                // if invalid or change direction or edge too long
                if ((!t.valid()) || (t.positive ^ positive) || bade(current_turn_index))
                {
                    try_push();
                    break;
                }
                ++current_turn_index;
            }

            if (current_turn_index == turn_count)
            {
                --current_turn_index;
                try_push();
                ++current_turn_index;
            }
        }

        return result;
    }

    bool near(circle a, circle b)
    {
        auto minr = std::min(r(a), r(b));
        auto maxr = std::max(r(a), r(b));
        BOOST_ASSERT(maxr > 1e-8);

        return
            minr > (1 - COMBINE_THRESHOLD_RELATIVE) * maxr &&
            length(cml::vector2d(x(a) - x(b), y(a) - y(b))) <= COMBINE_THRESHOLD_RELATIVE * maxr;
    }

    bool could_combine(const arc &a, const arc &b)
    {
        // too restrict
        //return
        //    std::abs(x(a.guess) - x(b.guess)) <= COMBINE_THRESHOLD &&
        //    std::abs(y(a.guess) - y(b.guess)) <= COMBINE_THRESHOLD &&
        //    std::abs(r(a.guess) - r(b.guess)) <= COMBINE_THRESHOLD;

        return near(a.guess, b.guess);
    }

    struct circle_with_info
    {
        cvcourse::circle circle;
        double mse;
    };
    typedef std::vector<circle_with_info> circle_with_info_container;

    circle_with_info_container combine_arcs(
        const arc_container &arcs,
        boost::optional<double> no_cover_radio
        )
    {
        typedef circle_with_info_container result_type;

        if (arcs.empty()) return result_type();

#if 0
        {
            const int arc_count = arcs.size();
            std::vector<int> mark(arc_count, -1);
            int mark_kind_count = 0;

            // make groups
            for (int i = 0; i != arc_count; ++i)
            {
                if (~mark[i]) continue;
                mark[i] = mark_kind_count;
                for (int j = i + 1; j != arc_count; ++j)
                {
                    if (~mark[j]) continue;
                    if (could_combine(arcs[i], arcs[j]))
                    {
                        mark[j] = mark_kind_count;
                    }
                }
                ++mark_kind_count;
            }

            circle_container result;

            // take average
            for (int kind = 0; kind != mark_kind_count; ++kind)
            {
                std::vector<int> indics = 
                    boost::irange<int>(0, arc_count) | 
                    bada::filtered([&](int i){ return mark[i] == kind; }) |
                    boost::as_container;

                auto cover = boost::accumulate(
                    indics | bada::transformed([&](int i){ return arcs[i].cover; }),
                    0.0
                    ) / (2 * pi);
                if (cover >= COVER_THRESHOLD)
                {
                    result.push_back(
                        boost::accumulate(
                        indics | bada::transformed([&](int i){ return arcs[i].guess; }),
                        circle(0, 0, 0)
                        ) * (1.0 / boost::size(indics))
                        );
                }
            }

            return result;
        }
#endif
        /// more flexible, marked arc can be reused
        {
            const int arc_count = arcs.size();
            std::vector<int> mark(arc_count, -1);
            int mark_kind_count = 0;

            result_type result;

            // make groups
            for (int i = 0; i != arc_count; ++i)
            {
                if (~mark[i]) continue;
                mark[i] = mark_kind_count;
                for (int j = i + 1; j != arc_count; ++j)
                {
                    //if (~mark[j]) continue;
                    if (could_combine(arcs[i], arcs[j]))
                    {
                        mark[j] = mark_kind_count;
                    }
                }

                {
                    const int kind = mark_kind_count;

                    std::vector<int> indics = 
                        boost::irange<int>(0, arc_count) | 
                        bada::filtered([&](int i){ return mark[i] == kind; }) |
                        boost::as_container;

                    //auto cover = boost::accumulate(
                    //    indics | bada::transformed([&](int i){ return arcs[i].cover; }),
                    //    0.0
                    //    ) / (2 * pi);
                    //qDebug() << "?";
                    auto cover = covered(indics | bada::transformed([&](int i){ return arcs[i].interval; }) | boost::as_container) / (2 * pi);
                    //qDebug() << "!";
                    if (cover >= no_cover_radio.get_value_or(COVER_THRESHOLD))
                    {
                        //// collect points from arcs
                        //std::vector<cml::vector2d> points;
                        //for each (auto i in indics) ba::push_back(points).range(arcs[i].points);

                        //qDebug() << "b4bag" << QVector<int>::fromStdVector(indics);
                        //// calc mse
                        //auto ret = fit_circle::go(points);
                        auto ret = bag(indics | bada::transformed([&](int i){ return arcs[i].points; }),
                            no_cover_radio.get_value_or(COVER_THRESHOLD) * 2 * pi);
                        //qDebug() << "aftbag" << ret.mse;

                        //if (ret.mse <= MSE_THRESHOLD)
                        /// TODO
                        //if (good_mse(ret.mse, ret.radius))
                        {
                            result_type::value_type elem = { circle(ret.center[0], ret.center[1], ret.radius), ret.mse };
                            result.push_back(elem);
                        }
                    }

                    //boost::fill(mark, -1);
                }

                ++mark_kind_count;
            }

            return result;
        }
    }

    circle_container select_best_circle(const circle_with_info_container &cis)
    {
        const int n = cis.size();
        std::vector<bool> bad(n, false);

        for each (auto i in boost::irange<int>(0, n))
        {
            if (bad[i]) continue;
            for each (auto j in boost::irange<int>(i + 1, n))
            {
                if (bad[j]) continue;
                if (near(cis[i].circle, cis[j].circle))
                {
                    if (cis[i].mse < cis[j].mse) {
                        bad[j] = true;
                    } else {
                        bad[i] = true;
                    }
                }
            }
        }

        return
            boost::irange<int>(0, n) |
            bada::filtered([&](int i){ return !bad[i]; }) |
            bada::transformed([&](int i){ return cis[i].circle; }) |
            boost::as_container;
    }
}

cvcourse::circle_container cvcourse::edcircles(
    const edge_segment_container &es,
    const boost::optional<double> no_cover_radio
    )
{
    /// calculate arcs
    arc_container arcs;
    for each (auto e in es)
    {
        ba::push_back(arcs).range(calculate_arcs(e));
    }

    return select_best_circle(combine_arcs(arcs, no_cover_radio));
}

cvcourse::edge_segment cvcourse::contour_to_edge_segment(const contour &c)
{
    std::vector<cv::Point> result;
    cv::approxPolyDP(c, result, CONTOUR_APPROXIMATION_THRESHOLD, true);

    if (result.size() < 3) return edge_segment();

    // append start and end point
    auto b = result.back();
    result.push_back(result.front());
    boost::reverse(result);
    result.push_back(b);
    boost::reverse(result);

    return result;
}

#include "algo.hpp"

cvcourse::adaptive_edcircles::adaptive_edcircles(const cv::Mat3b &image, const params &para)
{
    auto canvas = image.clone();
    auto make_es = [&]()->edge_segment_container
    {
        return find_contours(thresh(canvas)) |
            bada::transformed(&contour_to_edge_segment) |
            bada::filtered([](const edge_segment &e){ return !e.empty(); }) |
            boost::as_container;
    };

    scale = 1;
    {
        qDebug() << "no cover radio:" << para.no_cover_radio;
        auto cs = edcircles(make_es(), para.no_cover_radio);
        if (!cs.empty()) {
            auto largest = *boost::max_element(cs | bada::transformed([&](circle c){ return c(2); }));
            scale = IDEAL_RADIUS / largest;
            qDebug() << "scale:" << scale;
            cv::resize(canvas, canvas, cv::Size(), scale, scale);
        } else {
            qDebug() << "no circle found, no scale";
        }
    }

    edge_segments = make_es();
    circles = edcircles(edge_segments, para.no_cover_radio) |
        bada::transformed([&](const circle &c){ return c * (1 / scale); }) |
        boost::as_container;
    edge_segments = edge_segments |
        bada::transformed([&](const edge_segment &es)->edge_segment{
            edge_segment result;
            for each (auto e in es) result.push_back(e * (1 / scale));
            return result;
            }) |
        boost::as_container;
}
