#pragma once
#if !defined(__QTUTILS_HPP_2011121400043__)
#define __QTUTILS_HPP_2011121400043__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2011-12-14
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <memory> // unique_ptr

#include <QWidget>

class QScrollArea;

namespace cvcourse
{
    /// put widget into scroll area
    std::unique_ptr<QScrollArea> scrolled(QWidget &widget, bool wheel_disabled = false);
}

#endif // __QTUTILS_HPP_2011121400043__
