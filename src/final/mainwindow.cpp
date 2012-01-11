/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include <memory> // unique_ptr

#include <opencv2/opencv.hpp>

#include <QPushButton>
#include <QHBoxLayout>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>
#include <ans/alpha/method.hpp>

#include <boost/assert.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range.hpp>
#include <boost/assign.hpp>
#include <boost/range/to_container.hpp>

#include "mainwindow.hpp"
#include "yacvwindow.hpp"
#include "yacvwidget.hpp"
#include "algo.hpp"
#include "candidate_radius.hpp"
#include "ed.hpp"

#include <QWeakPointer>
#include <QDebug>
#include <qwt_slider.h>
#include <qwt_scale_engine.h>

#include <QDoubleSpinBox>
#include <QVariant>
#include <QLabel>


using namespace cvcourse;
namespace bada = boost::adaptors;

typedef std::unique_ptr<radius_description> radius_description_ptr;
typedef std::vector<cv::Vec3f> circle_container;
typedef QWeakPointer<cvcourse::yacvwindow> window_ptr;

struct cvcourse::mainwindow::impl
{
    cvcourse::yacvwindow *rawwin;
    QWeakPointer<cvcourse::yacvwindow> hough_window;
    circle_container circles;

    QWeakPointer<cvcourse::yacvwindow> preprocessed_window;
    std::vector<radius_description_ptr> radius_descriptions;

    window_ptr contour_approximation_window;
    window_ptr edcircles_window;
};

namespace
{
    const double IDEAL_RADIUS = 50;

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

                //if (false) {
                //    //qDebug() << field("dilate_kernel_radio");

                //    preprocess_params params = {
                //        static_cast<int>(field("dilate_kernel_radio").toDouble())
                //        };
                //    auto preprocessed_image = cvcourse::preprocess(
                //        input,
                //        params
                //        );

                //    //qDebug() << "preprocessed";

                //    if (!self->preprocessed_window.data())
                //    {
                //        auto ya = new generated_yacvwindow(this);
                //        ya->setWindowTitle(tr("preprocessed"));
                //        self->preprocessed_window = ya;
                //    }
                //    
                //    auto ya = self->preprocessed_window.data();
                //    ya->plot().set(preprocessed_image);
                //    ya->show();
                //}

#if 0
                {
                    auto source = input.clone();
                    std::vector<contour> contours;

                    {
                        contours = find_contours(make_background_black(thresh(source)));
                        auto circle_contours = boost::copy_range<decltype(contours)>(contours | bada::filtered(&candidate_contour));

                        //qDebug() << "candidate contours:" << contours.size();

                        // calculate radius description
                        {
                            auto radius = boost::copy_range<std::vector<double> >(circle_contours | bada::transformed(&contour_radius));
                            std::vector<double> radios;
                            for (int i = 0, n = field("kind").toInt(); i != n; ++i)
                            {
                                radios.push_back((i + 1.0) / n);
                            }

                            auto candis = candidate_radius(radius, radios);

                            self->radius_descriptions.clear();
                            for each (auto candi in candis | bada::filtered([](const radius_container &rs){ return !rs.empty(); }))
                            {
                                self->radius_descriptions.push_back(radius_description_ptr(new radius_description(candi)));
                                auto &desc = self->radius_descriptions.back();
                                //qDebug() << desc->best_guess() << desc->lower_bound() << desc->upper_bound();
                            }
                        }
                    }

                    cv::Mat3b preprocessed_image(source.size(), 0);
                    cv::drawContours(preprocessed_image, contours, -1, cv::Scalar(255, 255, 255), CV_FILLED);

                    //// scale image
                    //{
                    //    double scale = 100.0 / self->radius_descriptions.back()->best_guess();

                    //    std::vector<radius_description_ptr> t;
                    //    for each (auto &desc in self->radius_descriptions)
                    //    {
                    //        t.push_back(radius_description_ptr(new radius_description(desc->scale(scale))));
                    //    }
                    //    self->radius_descriptions = std::move(t);

                    //    cv::resize(preprocessed_image, preprocessed_image, cv::Size(), scale, scale);
                    //}

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
#endif

#if 0
                {
                    auto source = input.clone();
                    //cv::GaussianBlur(source, source, cv::Size(9, 9), 2, 2);
                    make_background_black(thresh(source)).copyTo(source);
                    //auto elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
                    //cv::dilate(source, source, elem);
                    auto contours = find_contours(source);
                    cv::cvtColor(source, source, CV_GRAY2BGR);

                    //auto source = input.clone();
                    //auto contours = find_contours(make_background_black(thresh(source)));
                    edge_segment_container es = contours |
                        bada::transformed(&contour_to_edge_segment) |
                        bada::filtered([](const edge_segment &e){ return !e.empty(); }) |
                        boost::to_container;

                    {
                        if (!self->contour_approximation_window.data())
                        {
                            auto ya = new generated_yacvwindow(this);
                            ya->setWindowTitle(tr("contour approximation"));
                            self->contour_approximation_window = ya;
                        }
                        auto ya = self->contour_approximation_window.data();
                        auto approximation = source.clone();
                        cv::polylines(approximation, es, true, cv::Scalar(255), 2);
                        ya->plot().set(approximation);
                        ya->show();
                    }

                    //qDebug() << "well";
                    auto cs = edcircles(es);
                    //qDebug() << cs.size();
                    {
                        if (!self->edcircles_window.data())
                        {
                            auto ya = new generated_yacvwindow(this);
                            ya->setWindowTitle(tr("edcircles"));
                            self->edcircles_window = ya;
                        }
                        auto ya = self->edcircles_window.data();
                        auto ed = source.clone();
                        for each (auto c in cs)
                        {
                            cv::circle(ed, cv::Point(cvRound(c(0)), cvRound(c(1))), cvRound(c(2)), cv::Scalar(0, 0, 255), 2);
                        }
                        ya->plot().set(ed);
                        ya->show();
                    }
                }
#endif
                {
                    auto source = input.clone();

                    {
                        auto contours = find_contours(make_background_black(thresh(source)));
                        auto circle_contours = boost::copy_range<decltype(contours)>(contours | bada::filtered(&candidate_contour));

                        //qDebug() << "candidate contours:" << contours.size();

                        // calculate radius description
                        {
                            auto radius = boost::copy_range<std::vector<double> >(circle_contours | bada::transformed(&contour_radius));
                            std::vector<double> radios;
                            for (int i = 0, n = field("kind").toInt(); i != n; ++i)
                            {
                                radios.push_back((i + 1.0) / n);
                            }

                            auto candis = candidate_radius(radius, radios);
                            //qDebug() << "well";

                            self->radius_descriptions.clear();
                            for each (auto candi in candis | bada::filtered([](const radius_container &rs){ return !rs.empty(); }))
                            {
                                self->radius_descriptions.push_back(radius_description_ptr(new radius_description(candi)));
                                auto &desc = self->radius_descriptions.back();
                                //qDebug() << desc->best_guess() << desc->lower_bound() << desc->upper_bound();
                            }
                        }
                    }

                    /// scale
                    if (!self->radius_descriptions.empty())
                    {
                        auto &raw = self->radius_descriptions.back();
                        const double scale = IDEAL_RADIUS / raw->best_guess();
                        qDebug() << "scale:" << scale;
                        cv::resize(source, source, cv::Size(), scale, scale);
                    }

                    /// ed
                    {
                        auto contours = find_contours(make_background_black(thresh(source)));
                        //cv::GaussianBlur(source, source, cv::Size(9, 9), 2, 2);
                        //make_background_black(thresh(source)).copyTo(source);
                        ////auto elem = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
                        ////cv::dilate(source, source, elem);
                        //auto contours = find_contours(source);
                        //cv::cvtColor(source, source, CV_GRAY2BGR);

                        //auto source = input.clone();
                        //auto contours = find_contours(make_background_black(thresh(source)));
                        edge_segment_container es = contours |
                            bada::transformed(&contour_to_edge_segment) |
                            bada::filtered([](const edge_segment &e){ return !e.empty(); }) |
                            boost::to_container;

                        {
                            if (!self->contour_approximation_window.data())
                            {
                                auto ya = new generated_yacvwindow(this);
                                ya->setWindowTitle(tr("contour approximation"));
                                self->contour_approximation_window = ya;
                            }
                            auto ya = self->contour_approximation_window.data();
                            auto app = make_background_black(thresh(source));
                            cv::cvtColor(app, app, CV_GRAY2BGR);
                            cv::polylines(app, es, true, cv::Scalar(255), 2);
                            ya->plot().set(app);
                            ya->show();

                            //{
                            //    auto ya = new generated_yacvwindow(this);
                            //    ya->setWindowTitle("asdfsadfs");
                            //    auto g = source.clone();
                            //    cv::cvtColor(g, g, CV_BGR2GRAY);
                            //    cv::Canny(g, g, 100, 200);
                            //    cv::cvtColor(g, g, CV_GRAY2BGR);
                            //    ya->plot().set(g);
                            //    ya->show();
                            //}
                        }

                        //qDebug() << "well";
                        auto cs = edcircles(es);
                        //qDebug() << cs.size();
                        {
                            if (!self->edcircles_window.data())
                            {
                                auto ya = new generated_yacvwindow(this);
                                ya->setWindowTitle(tr("edcircles"));
                                self->edcircles_window = ya;
                            }
                            auto ya = self->edcircles_window.data();
                            auto ed = source.clone();
                            for each (auto c in cs)
                            {
                                cv::circle(ed, cv::Point(cvRound(c(0)), cvRound(c(1))), cvRound(c(2)), cv::Scalar(0, 0, 255), 2);
                            }
                            ya->plot().set(ed);
                            ya->show();
                        }
                    }
                }
            }
        };
    }
    typedef ans::alpha::functional::method<detail::page_method<input_page> > input_page_method;
    typedef ans::alpha::functional::method<detail::page_method<hough_page> > hough_page_method;
    typedef ans::alpha::functional::method<detail::page_method<match_page> > match_page_method;
    typedef ans::alpha::functional::method<detail::page_method<preprocess_page> > preprocess_page_method;
}

cvcourse::mainwindow::mainwindow() : self(use_default_ctor)
{
    addPage(new input_page(this));
    addPage(new preprocess_page(this));
    addPage(new hough_page(this));
    //addPage(new match_page(this));
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

cvcourse::hough_page::hough_page(QWidget *parent) : base_type(parent)
{
    //hough_page_method method;
    //auto main = &method(this)->main();
    //auto self = &method(this)->self();

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

    auto input = method(this)->preprocessed_image();
    BOOST_ASSERT(!input.empty());

    //double high = field("canny_high").toDouble();
    //double dist = input.size().height * field("dist").toDouble();

    circle_container circles;
    for each (auto &raw in self->radius_descriptions)
    {
        const double scale = 100.0 / raw->best_guess();
        auto rd = raw->scale(scale);
        detect_circles_params params = {
            //field("dp").toDouble(),
            //high,
            //dist,
            //field("accumulator_threshold").toDouble(),
            rd.lower_bound(),
            rd.upper_bound(),
            };
        cv::Mat3b scaled;
        cv::resize(input, scaled, cv::Size(), scale, scale);
        auto ret = detect_disks(scaled, params);
        for each (auto c in ret)
        {
            for (int i = 0; i < 3; ++i)
            {
                c(i) /= scale;
            }
            circles.push_back(c);
        }
        //boost::assign::push_back(circles).range(ret);
        //input = draw_cycles(input, self->circles);
    }

    //qDebug() << circles.size();
    
    self->circles = merge_circles(circles);
    auto result = draw_circles(input, self->circles);

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
            cv::circle(mask, center, radius, cv::Scalar(255));
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
    //auto self = &method(this)->self();

    auto mlay = new QVBoxLayout(this);
    setLayout(mlay);

#if 0
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
#endif

    {
        auto lay = new QHBoxLayout;
        mlay->addLayout(lay);

        auto label = new QLabel("dilate kernel radio", this);
        lay->addWidget(label);

        auto spin = new QSpinBox(this);
        lay->addWidget(spin);
        spin->setMinimum(1);
        spin->setMaximum(10);
        spin->setSingleStep(1);
        registerField("kind", spin, "value", "valueChanged()");

        spin->setValue(1);
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
