#include <vector>
#include <memory>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/assert.hpp>

#include <QSharedPointer>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>

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
    QDialog param_dialog;
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

        void initialize_param_dialog()
        {
            auto dialog = &impl->param_dialog;

            dialog->setWindowTitle(tr("parameters"));

            auto lay = new QVBoxLayout;
            impl->param_dialog.setLayout(lay);

            auto grid = new QGridLayout;
            lay->addLayout(grid);
            {
                auto row = grid->rowCount();
                auto label = new QLabel(tr("low"));
                auto edit = new QLineEdit;
                edit->setObjectName("low");
                edit->setText("35");
                label->setBuddy(edit);
                grid->addWidget(label, row, 0);
                grid->addWidget(edit, row, 1);
            }
            {
                auto row = grid->rowCount();
                auto sub = new QHBoxLayout;
                auto label = new QLabel(tr("high"));
                auto edit = new QLineEdit;
                edit->setObjectName("high");
                edit->setText("70");
                label->setBuddy(edit);
                grid->addWidget(label, row, 0);
                grid->addWidget(edit, row, 1);
            }
            {
                auto row = grid->rowCount();
                auto sub = new QHBoxLayout;
                auto label = new QLabel(tr("aperture"));
                auto edit = new QLineEdit;
                edit->setObjectName("aperture");
                edit->setText("3");
                label->setBuddy(edit);
                grid->addWidget(label, row, 0);
                grid->addWidget(edit, row, 1);
            }

            auto box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            lay->addWidget(box);
            connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
            connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
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

            initialize_param_dialog();
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
    if (!method(this)->input().empty())
    {
        if (impl->param_dialog.exec() == QDialog::Accepted)
        {
            cv::Mat input, result;
            cv::cvtColor(method(this)->input(), input, CV_BGR2GRAY);
            cv::cvtColor(canny(input, 
                impl->param_dialog.findChild<QLineEdit*>("low")->text().toDouble(),
                impl->param_dialog.findChild<QLineEdit*>("high")->text().toDouble(),
                impl->param_dialog.findChild<QLineEdit*>("aperture")->text().toInt()
                ), result, CV_GRAY2BGR);
            method(this)->set_output(result);
        }
    }
}

