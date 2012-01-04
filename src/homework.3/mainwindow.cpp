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

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>
#include <ans/alpha/method.hpp>

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

namespace
{
    struct mainwindow_method : cvcourse::mainwindow
    {
        void create_actions()
        {
            {
                auto p = action_ptr(new QAction(tr("&Open"), this), &QObject::deleteLater);
                connect(p.data(), SIGNAL(triggered()), this, SLOT(open()));
                impl->actions.push_back(p);
            }
            {
                auto p = action_ptr(new QAction(tr("&Detect edge"), this), &QObject::deleteLater);
                connect(p.data(), SIGNAL(triggered()), this, SLOT(detect()));
                impl->actions.push_back(p);
            }
        }

        void create_menus()
        {
            auto p = menuBar()->addMenu(tr("&Main"));
            boost::for_each(impl->actions, [&](action_ptr a){ p->addAction(a.data()); });
        }

        void initialize_layout()
        {
            auto input = scrolled(impl->input, true);
            auto output = scrolled(impl->output, true);

            input->setMinimumSize(300, 400);
            input->setAlignment(Qt::AlignCenter);
            output->setMinimumSize(300, 400);
            output->setAlignment(Qt::AlignCenter);
            
            auto box = new QHBoxLayout(this);
            box->addWidget(input.release());
            box->addWidget(output.release());
            auto w = new QWidget;
            w->setLayout(box);
            setCentralWidget(w);
        }

        void set_input(cv::Mat mat)
        {
            impl->input.set_mat(mat);
        }

        cv::Mat input() const { return impl->input.mat(); }

        void set_output(cv::Mat mat)
        {
            impl->output.set_mat(mat);
        }

        cv::Mat output() const { return impl->output.mat(); }
    };

    ans::alpha::functional::method<mainwindow_method> method;
}

cvcourse::mainwindow::mainwindow() : impl(use_default_ctor())
{
    method(this)->initialize_layout();
    method(this)->create_actions();
    method(this)->create_menus();
}

void cvcourse::mainwindow::open()
{
    impl->input.load();
}

void cvcourse::mainwindow::detect()
{
    if (!method(this)->input().empty()) {
        cv::Mat3b input, result;
        cv::cvtColor(method(this)->input(), input, CV_BGR2RGB);
        cv::cvtColor(canny(input, 0, 0, 0), result, CV_RGB2BGR);
        method(this)->set_output(result);
    }
}

