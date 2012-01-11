/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-09
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QDebug>

namespace boost
{
    void assertion_failed(char const * expr, char const * function, char const * file, long line)
    {
        qDebug() << expr;
        qDebug() << function;
        qDebug() << file;
        qDebug() << line;
    }
}