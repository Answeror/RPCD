/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QWeakPointer>

#include <boost/assert.hpp>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>

#include "yacvwindow.hpp"
#include "yacvwidget.hpp"

struct cvcourse::yacvwindow::impl
{
    cvcourse::yacvwidget *yacv;
    QWeakPointer<QToolButton> load_button;
};

namespace
{
    const ans::alpha::pimpl::use_default_ctor use_default_ctor;
}

cvcourse::yacvwindow::yacvwindow(QWidget *parent) : QMainWindow(parent), self(use_default_ctor)
{
    resize(400, 300);

    self->yacv = new yacvwidget(this);
    setCentralWidget(self->yacv);

    auto toolbar = new yacvtoolbar(this);
    addToolBar(toolbar);

    auto loadbtn = new QToolButton(toolbar);
    loadbtn->setText("load");
    loadbtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(loadbtn);
    connect(loadbtn, SIGNAL(clicked()), self->yacv, SLOT(load()));

    self->load_button = loadbtn;

    auto savebtn = new QToolButton(toolbar);
    savebtn->setText("save");
    savebtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(savebtn);
    connect(savebtn, SIGNAL(clicked()), self->yacv, SLOT(save()));

    toolbar->addSeparator();

    auto actualbtn = new QToolButton(toolbar);
    actualbtn->setText("1:1");
    actualbtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(actualbtn);
    connect(actualbtn, SIGNAL(clicked()), self->yacv, SLOT(show_actual_size()));

    auto rescaleBox = new QComboBox(toolbar);
    rescaleBox->setEditable(false);
    rescaleBox->insertItem(yacvwidget::NONE, "none");
    rescaleBox->insertItem(yacvwidget::FIXED, "fixed");
    rescaleBox->insertItem(yacvwidget::FITTING, "fitting");
    rescaleBox->insertItem(yacvwidget::EXPANDING, "expanding");
    connect(rescaleBox, SIGNAL(activated(int)), self->yacv, SLOT(set_rescale_mode(int)));
    connect(self->yacv, SIGNAL(rescale_mode_changed(int)), rescaleBox, SLOT(setCurrentIndex(int)));
    toolbar->addWidget(new QLabel("rescale mode: ", toolbar));
    toolbar->addWidget(rescaleBox);

    //rescaleBox->setCurrentIndex(yacvwidget::EXPANDING);

    //toolbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

const cvcourse::yacvwidget& cvcourse::yacvwindow::plot() const
{
    BOOST_ASSERT(self->yacv);
    return *self->yacv;
}

cvcourse::yacvwidget& cvcourse::yacvwindow::plot()
{
    return const_cast<yacvwidget&>(static_cast<const yacvwindow*>(this)->plot());
}

void cvcourse::yacvwindow::enable_load()
{
    if (auto p = self->load_button.data())
    {
        p->show();
    }
}

void cvcourse::yacvwindow::disable_load()
{
    if (auto p = self->load_button.data())
    {
        p->hide();
    }
}

cvcourse::yacvtoolbar::yacvtoolbar(QWidget *parent) : QToolBar(parent)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

QSize cvcourse::yacvtoolbar::sizeHint() const 
{
    auto hint = QToolBar::sizeHint();
    if (auto p = qobject_cast<yacvwindow*>(parentWidget()))
    {
        if (auto yacv = qobject_cast<yacvwidget*>(p->centralWidget()))
        {
            hint.setWidth(yacv->sizeHint().width());
        }
    }
    return hint;
}

cvcourse::generated_yacvwindow::generated_yacvwindow(QWidget *parent) : base_type(parent)
{
    disable_load();
}
