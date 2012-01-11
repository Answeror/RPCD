#ifdef _MSC_VER
	#pragma once
#endif
#ifndef __MAINWINDOW_HPP_20120106234635__
#define __MAINWINDOW_HPP_20120106234635__

/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

//#include <QDialog>
#include <QWizard>
#include <QWizardPage>

#include <ans/alpha/pimpl.hpp>

namespace cvcourse
{
    class mainwindow : public QWizard
    {
        Q_OBJECT

    public:
        typedef mainwindow this_type;
        typedef QWizard base_type;

    public:
        mainwindow();

    public:
        struct impl;
        ans::alpha::pimpl::unique<impl> self;
    };

    template<class Derived>
    struct page
    {
        typedef Derived this_type;
        typedef QWizardPage base_type;
    };

    class input_page : public QWizardPage, public page<input_page>
    {
        Q_OBJECT

    public:
        input_page(QWidget *parent = nullptr);

    public Q_SLOTS:
        void show_input_window();

    public:
        void initializePage() override;
        void setVisible(bool visible) override;
    };

    class preprocess_page : public QWizardPage, public page<preprocess_page>
    {
        Q_OBJECT

    public:
        preprocess_page(QWidget*);

    public Q_SLOTS:
        void preprocess();
        void iterate();

    public:
        void initializePage() override;
        void cleanupPage() override;
        void setVisible(bool visible) override;
    };

    class hough_page : public QWizardPage, public page<hough_page>
    {
        Q_OBJECT

    public:
        hough_page(QWidget*);

    public Q_SLOTS:
        void hough();

    public:
        void initializePage() override;
        void cleanupPage() override;
    };

    class match_page : public QWizardPage, public page<match_page>
    {
        Q_OBJECT

    public:
        match_page(QWidget*);

    public:
        void initializePage() override;
    };
}

#endif // __MAINWINDOW_HPP_20120106234635__
