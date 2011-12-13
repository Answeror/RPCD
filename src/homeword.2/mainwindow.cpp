#include <vector>
#include <memory>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/assert.hpp>

#include <QSharedPointer>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QHBoxLayout>
#include <QScrollArea>

#include <ans/beta/pimpl.hpp>
#include <ans/beta/pimpl_impl.hpp>

#include <opencv2/opencv.hpp>

#include "mainwindow.hpp"
#include "cvmatwidget.hpp"
#include "algo.hpp"
#include "qtutils.hpp"

typedef QSharedPointer<QAction> action_ptr;
typedef QWeakPointer<QMenu> menu_ptr;

struct cvcourse::mainwindow::data
{
    std::vector<action_ptr> actions;
    cvcourse::cvmatwidget output;
    cvcourse::cvmatwidget input;
    cvcourse::cvmatwidget reference;
};

struct cvcourse::mainwindow::method : cvcourse::mainwindow
{
    void create_actions()
    {
        {
            auto p = action_ptr(new QAction(tr("&Open input"), this), &QObject::deleteLater);
            connect(p.data(), SIGNAL(triggered()), this, SLOT(open_input()));
            impl.data().actions.push_back(p);
        }
        {
            auto p = action_ptr(new QAction(tr("&Open reference"), this), &QObject::deleteLater);
            connect(p.data(), SIGNAL(triggered()), this, SLOT(open_reference()));
            impl.data().actions.push_back(p);
        }
        {
            auto p = action_ptr(new QAction(tr("&Color transfer"), this), &QObject::deleteLater);
            connect(p.data(), SIGNAL(triggered()), this, SLOT(transfer()));
            impl.data().actions.push_back(p);
        }
    }

    void create_menus()
    {
        auto p = menuBar()->addMenu(tr("&Main"));
        boost::for_each(impl.data().actions, [&](action_ptr a){ p->addAction(a.data()); });
    }

    void initialize_layout()
    {
        auto input = scrolled(impl.data().input, true);
        auto reference = scrolled(impl.data().reference, true);
        auto output = scrolled(impl.data().output, true);

        input->setMinimumSize(300, 400);
        input->setAlignment(Qt::AlignCenter);
        reference->setMinimumSize(300, 400);
        reference->setAlignment(Qt::AlignCenter);
        output->setMinimumSize(300, 400);
        output->setAlignment(Qt::AlignCenter);
        
        auto box = new QHBoxLayout(this);
        box->addWidget(input.release());
        box->addWidget(reference.release());
        box->addWidget(output.release());
        auto w = new QWidget;
        w->setLayout(box);
        setCentralWidget(w);
    }

    void set_input(cv::Mat mat)
    {
        impl.data().input.set_mat(mat);
    }

    cv::Mat input() const { return impl.data().input.mat(); }

    void set_reference(cv::Mat mat)
    {
        impl.data().reference.set_mat(mat);
    }

    cv::Mat reference() const { return impl.data().reference.mat(); }

    void set_output(cv::Mat mat)
    {
        impl.data().output.set_mat(mat);
    }

    cv::Mat output() const { return impl.data().output.mat(); }
};

cvcourse::mainwindow::mainwindow() : impl(use_default_ctor())
{
    impl.method(this).initialize_layout();
    impl.method(this).create_actions();
    impl.method(this).create_menus();
}

void cvcourse::mainwindow::open_input()
{
    impl.data().input.load();
}

void cvcourse::mainwindow::open_reference()
{
    impl.data().reference.load();
}

void cvcourse::mainwindow::transfer()
{
    if (!impl.method(this).input().empty() && !impl.method(this).reference().empty()) {
        cv::Mat3b in, ref, out;
        //cv::cvtColor(impl.method(this).input(), in, CV_BGR2HSV);
        //cv::cvtColor(impl.method(this).reference(), ref, CV_BGR2HSV);
        //cv::cvtColor(color_transfer(in, ref), out, CV_HSV2BGR);
        cv::cvtColor(impl.method(this).input(), in, CV_BGR2RGB);
        cv::cvtColor(impl.method(this).reference(), ref, CV_BGR2RGB);
        cv::cvtColor(reinhard(in, ref), out, CV_RGB2BGR);
        impl.method(this).set_output(out);
    }
}

