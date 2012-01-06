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

#include <boost/assert.hpp>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>

#include "yacvwindow.hpp"
#include "yacvwidget.hpp"

struct cvcourse::yacvwindow::impl
{
    cvcourse::yacvwidget *yacv;
};

namespace
{
    const ans::alpha::pimpl::use_default_ctor use_default_ctor;
}

cvcourse::yacvwindow::yacvwindow(QWidget *parent) : QMainWindow(parent), self(use_default_ctor)
{
    self->yacv = new yacvwidget(this);
    setCentralWidget(self->yacv);

    auto toolbar = new QToolBar(this);
    addToolBar(toolbar);

    auto loadbtn = new QToolButton(toolbar);
    loadbtn->setText("load");
    loadbtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(loadbtn);
    connect(loadbtn, SIGNAL(clicked()), self->yacv, SLOT(load()));

    auto savebtn = new QToolButton(toolbar);
    savebtn->setText("save");
    savebtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(savebtn);
    connect(savebtn, SIGNAL(clicked()), self->yacv, SLOT(save()));

    toolbar->addSeparator();

    auto rescaleBox = new QComboBox(toolbar);
    rescaleBox->setEditable(false);
    rescaleBox->insertItem(yacvwidget::EXPANDING, "expanding");
    rescaleBox->insertItem(yacvwidget::FITTING, "fitting");
    rescaleBox->insertItem(yacvwidget::FIXED, "fixed");
    rescaleBox->insertItem(yacvwidget::NONE, "none");
    connect(rescaleBox, SIGNAL(activated(int)), self->yacv, SLOT(set_rescale_mode(int)));
    toolbar->addWidget(new QLabel("rescale mode: ", toolbar));
    toolbar->addWidget(rescaleBox);
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
