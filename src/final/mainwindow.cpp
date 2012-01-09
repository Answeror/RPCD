/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <opencv2/opencv.hpp>

#include <QPushButton>
#include <QHBoxLayout>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>
#include <ans/alpha/method.hpp>

#include <boost/assert.hpp>

#include "mainwindow.hpp"
#include "yacvwindow.hpp"
#include "yacvwidget.hpp"
#include "algo.hpp"

#include <QWeakPointer>
#include <QDebug>

struct cvcourse::mainwindow::impl
{
    cvcourse::yacvwindow *rawwin;
    QWeakPointer<cvcourse::yacvwindow> hough_window;
    std::vector<cv::Vec3f> circles;

    QWeakPointer<cvcourse::yacvwindow> preprocessed_window;
};

namespace cvcourse { namespace
{
    const ans::alpha::pimpl::use_default_ctor use_default_ctor;

    namespace detail
    {
        template<class T>
        struct page_method : T
        {
            mainwindow& main() const
            {
                auto p = qobject_cast<mainwindow*>(wizard());
                BOOST_ASSERT(p);
                return *p;
            }

            mainwindow::impl& self() 
            {
                return *main().self;
            }

            const mainwindow::impl& self() const
            {
                return *main().self;
            }

            cv::Mat input_image() const
            {
                if (auto p = self().rawwin)
                {
                    return p->plot().get();
                }
                return cv::Mat();
            }

            cv::Mat preprocessed_image() const
            {
                if (auto p = self().preprocessed_window.data())
                {
                    return p->plot().get();
                }
                return cv::Mat();
            }

            void preprocess(const cv::Mat3b &input)
            {
                const preprocess_page_method method;
                auto self = &method(this)->self();

                BOOST_ASSERT(!input.empty());

                //qDebug() << field("dilate_kernel_radio");

                preprocess_params params = {
                    static_cast<int>(field("dilate_kernel_radio").toDouble())
                    };
                auto preprocessed_image = cvcourse::preprocess(
                    input,
                    params
                    );

                //qDebug() << "preprocessed";

                if (!self->preprocessed_window.data())
                {
                    auto ya = new generated_yacvwindow(this);
                    ya->setWindowTitle(tr("preprocessed"));
                    self->preprocessed_window = ya;
                }
                
                auto ya = self->preprocessed_window.data();
                ya->plot().set(preprocessed_image);
                ya->show();
            }
        };
    }
    typedef ans::alpha::functional::method<detail::page_method<input_page> > input_page_method;
    typedef ans::alpha::functional::method<detail::page_method<hough_page> > hough_page_method;
    typedef ans::alpha::functional::method<detail::page_method<match_page> > match_page_method;
    typedef ans::alpha::functional::method<detail::page_method<preprocess_page> > preprocess_page_method;
}}

cvcourse::mainwindow::mainwindow() : self(use_default_ctor)
{
    addPage(new input_page(this));
    addPage(new preprocess_page(this));
    addPage(new hough_page(this));
    addPage(new match_page(this));
}

cvcourse::input_page::input_page(QWidget *parent) : base_type(parent)
{
}

void cvcourse::input_page::initializePage()
{
    input_page_method method;
    auto main = &method(this)->main();
    auto self = &method(this)->self();

    self->rawwin = new yacvwindow(this);
    self->rawwin->setWindowTitle("raw");
    self->rawwin->show();
}

#include <qwt_slider.h>
#include <qwt_scale_engine.h>

#include <QDoubleSpinBox>
#include <QVariant>
#include <QLabel>

cvcourse::hough_page::hough_page(QWidget *parent) : base_type(parent)
{
    hough_page_method method;
    auto main = &method(this)->main();
    auto self = &method(this)->self();

    auto mainlay = new QVBoxLayout(this);
    setLayout(mainlay);

    auto gen = [&](const QString &name, double low, double high, double init)
    {
        auto lay = new QHBoxLayout;
        mainlay->addLayout(lay);

        QString labelname = name;
        labelname.replace("_", " ");
        auto label = new QLabel(labelname, this);
        lay->addWidget(label);

        auto box = new QDoubleSpinBox(this);
        lay->addWidget(box);
        box->setMinimum(low);
        box->setMaximum(high);
        registerField(name, box, "value", "valueChanged()");

        auto slider = new QwtSlider(this, Qt::Horizontal, 
            QwtSlider::BottomScale, QwtSlider::Groove);
        lay->addWidget(slider);
        slider->setHandleSize( 12, 25 );
        slider->setRange(low, high);

        connect(box, SIGNAL(valueChanged(double)), slider, SLOT(setValue(double)));
        connect(slider, SIGNAL(valueChanged(double)), box, SLOT(setValue(double)));

        box->setValue(init);
    };

    gen("dp", 1, 10, 1);
    gen("dist", 0, 1, 0.2);
    gen("canny_high", 1, 300, 200);
    gen("accumulator_threshold", 1, 300, 100);

    {
        auto lay = new QHBoxLayout;
        mainlay->addLayout(lay);

        auto calcbtn = new QPushButton("hough", this);
        lay->addWidget(calcbtn);
        connect(calcbtn, SIGNAL(clicked()), SLOT(hough()));
    }
}

void cvcourse::hough_page::initializePage()
{
}

void cvcourse::hough_page::hough()
{
    hough_page_method method;
    auto self = &method(this)->self();

    BOOST_ASSERT(!method(this)->input_image().empty());

    auto src = method(this)->input_image();

    double high = field("canny_high").toDouble();
    double dist = src.size().height * field("dist").toDouble();

    detect_circles_params params = {
        field("dp").toDouble(),
        high,
        dist,
        field("accumulator_threshold").toDouble()
        };
    self->circles = detect_disks(src, params);
    auto result = draw_cycles(src, self->circles);

    {
        auto name = "hough_window";
        generated_yacvwindow *ya = nullptr;
        if (!(ya = findChild<generated_yacvwindow*>(name)))
        {
            ya = new generated_yacvwindow(this);
            ya->setObjectName(name);
            ya->setWindowTitle("hough");
            self->hough_window = ya;
        }
        ya->plot().set(result);
        ya->show();
    }
}

void cvcourse::hough_page::cleanupPage()
{
    auto name = "hough_window";
    if (auto ya = findChild<generated_yacvwindow*>(name))
    {
        ya->hide();
    }
}

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <vector>

namespace pt = boost::property_tree;

namespace
{
    pt::ptree load_config()
    {
        pt::ptree conf;
        pt::read_xml("final.xml", conf);
        return conf;
    }

    std::vector<cv::Mat> load_templates()
    {
        std::vector<cv::Mat> result;
        auto conf = load_config();
        for each (auto &node in conf.get_child("final.templates"))
        {
            result.push_back(cv::imread(node.second.get_value<std::string>()));
        }
        return result;
    }
}

#include <boost/assert.hpp>

cvcourse::match_page::match_page(QWidget *parent) : base_type(parent) {}

void cvcourse::match_page::initializePage()
{
    match_page_method method;
    auto main = &method(this)->main();
    auto self = &method(this)->self();

    if (auto hough_window = self->hough_window.data())
    {
        auto ts = load_templates();
        for each (auto &t in ts)
        {
            auto ya = new generated_yacvwindow(this);
            ya->setWindowTitle("template");
            ya->plot().set(t);
            ya->show();
        }

        // target image
        auto scene = method(this)->input_image();

        // segmentation
        for each (auto c in self->circles)
        {
            cv::Point center(cvRound(c[0]), cvRound(c[1]));
            int radius = cvRound(c[2]);

            // circle outline
            cv::Mat1b mask(scene.size(), 0);
            circle(mask, center, radius, cv::Scalar(255));
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

            BOOST_ASSERT(contours.size() == 1);

            auto box = cv::boundingRect(contours.front());
            auto part = scene(box);

            //auto ya = new generated_yacvwindow(this);
            //ya->setWindowTitle("part");
            //ya->plot().set(part);
            //ya->show();
        }
    }
}

cvcourse::preprocess_page::preprocess_page(QWidget *parent) : base_type(parent)
{
    const preprocess_page_method method;
    auto self = &method(this)->self();

    auto mlay = new QVBoxLayout(this);
    setLayout(mlay);

    {
        auto lay = new QHBoxLayout;
        mlay->addLayout(lay);


        auto label = new QLabel("dilate kernel radio", this);
        lay->addWidget(label);

        auto box = new QDoubleSpinBox(this);
        lay->addWidget(box);
        box->setMinimum(0);
        box->setMaximum(100);
        box->setSingleStep(1);
        registerField("dilate_kernel_radio", box, "value", "valueChanged()");

        auto slider = new QwtSlider(this, Qt::Horizontal, 
            QwtSlider::NoScale, QwtSlider::Trough | QwtSlider::Groove );
        lay->addWidget(slider);
        slider->setRange(box->minimum(), box->maximum(), box->singleStep(), 5);

        connect(box, SIGNAL(valueChanged(double)), slider, SLOT(setValue(double)));
        connect(slider, SIGNAL(valueChanged(double)), box, SLOT(setValue(double)));

        box->setValue(2);
    }
    {
        auto lay = new QHBoxLayout;
        mlay->addLayout(lay);

        auto pre = new QPushButton("preprocess", this);
        lay->addWidget(pre);
        connect(pre, SIGNAL(clicked()), SLOT(preprocess()));

        auto ite = new QPushButton("iterate", this);
        lay->addWidget(ite);
        connect(ite, SIGNAL(clicked()), SLOT(iterate()));
    }
}

void cvcourse::preprocess_page::preprocess()
{
    const preprocess_page_method method;
    auto self = &method(this)->self();

    BOOST_ASSERT(!method(this)->input_image().empty());
    method(this)->preprocess(method(this)->input_image());
}

void cvcourse::preprocess_page::initializePage()
{
    //preprocess();
}

void cvcourse::preprocess_page::cleanupPage()
{
    const preprocess_page_method method;
    auto self = &method(this)->self();

    if (auto ya = self->preprocessed_window.data())
    {
        ya->hide();
    }
}

void cvcourse::preprocess_page::iterate()
{
    const preprocess_page_method method;
    auto self = &method(this)->self();

    BOOST_ASSERT(!method(this)->preprocessed_image().empty());
    method(this)->preprocess(method(this)->preprocessed_image());
}
