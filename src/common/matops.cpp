#include <boost/range.hpp>

#include "matops.hpp"

bool cvcourse::all(const cv::Mat &m)
{
    for each (auto e in boost::make_iterator_range(m.data, m.data + m.total() * m.elemSize()))
    {
        if (!e) return false;
    }
    return true;
}

bool cvcourse::any(const cv::Mat &m)
{
    for each (auto e in boost::make_iterator_range(m.data, m.data + m.total() * m.elemSize()))
    {
        if (e) return true;
    }
    return false;
}
