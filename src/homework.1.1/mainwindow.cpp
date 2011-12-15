#include <vector>
#include <memory>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/assert.hpp>

#include <QSharedPointer>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

#include "mainwindow.hpp"
#include "cvmatwidget.hpp"
#include "qtutils.hpp"

typedef QSharedPointer<QAction> action_ptr;
typedef QWeakPointer<QMenu> menu_ptr;

template<>
struct ans::pimpl<cvcourse::mainwindow>::data
{
    std::vector<action_ptr> actions;
    cvcourse::cvmatwidget canvas;
};

template<>
struct ans::pimpl<cvcourse::mainwindow>::method : cvcourse::mainwindow
{
    void create_actions()
    {
        {
            auto p = action_ptr(new QAction(tr("&Open"), this), &QObject::deleteLater);
            connect(p.data(), SIGNAL(triggered()), this, SLOT(open()));
            _data().actions.push_back(p);
        }
        {
            auto p = action_ptr(new QAction(tr("&Convert"), this), &QObject::deleteLater);
            connect(p.data(), SIGNAL(triggered()), this, SLOT(convert()));
            _data().actions.push_back(p);
        }
    }

    void create_menus()
    {
        auto p = menuBar()->addMenu(tr("&Main"));
        boost::for_each(_data().actions, [&](action_ptr a){ p->addAction(a.data()); });
    }

    void initialize_layout()
    {
        //this->setCentralWidget(&_data().canvas);
        this->setCentralWidget(resize_by_wheel(_data().canvas).release());
    }

    void set_image(cv::Mat mat)
    {
        _data().canvas.set_mat(mat);
    }

    cv::Mat image() const { return _data().canvas.mat(); }
};

cvcourse::mainwindow::mainwindow() : impl(use_default_ctor())
{
    _method(this).initialize_layout();
    _method(this).create_actions();
    _method(this).create_menus();
}

void cvcourse::mainwindow::open()
{
    _data().canvas.load();
}

void cvcourse::mainwindow::convert()
{
    if (!_method(this).image().empty()) {
        cv::Mat mat;
        cv::cvtColor(_method(this).image(), mat, CV_BGR2GRAY);
        cv::cvtColor(mat, mat, CV_GRAY2BGR);
        _method(this).set_image(mat);
    }
}
