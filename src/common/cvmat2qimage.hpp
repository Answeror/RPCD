#if !defined(__CVMAT2QIMAGE_HPP_2011112822745__)
#define __CVMAT2QIMAGE_HPP_2011112822745__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-11-28
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QImage>

#include <opencv2/core/core.hpp>

namespace cvcourse
{
    /**
     *  @param mat must be RGB format
     */
    inline QImage cvmat2qimage(cv::Mat mat)
    {
        return QImage(
            reinterpret_cast<const unsigned char*>(mat.data),
            mat.cols,
            mat.rows,
            mat.step,
            QImage::Format_RGB888
            );
    }
}

#endif // __CVMAT2QIMAGE_HPP_2011112822745__
