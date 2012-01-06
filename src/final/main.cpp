#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __MAIN_CPP_20120106232042__
#define __MAIN_CPP_20120106232042__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QApplication>

#include "mainwindow.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    cvcourse::mainwindow ya;
    ya.show();
    return app.exec();
}

#endif // __MAIN_CPP_20120106232042__
