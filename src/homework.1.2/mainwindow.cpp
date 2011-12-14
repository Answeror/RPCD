#include <vector>
#include <memory>
#include <algorithm>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/timer.hpp>

#include <QSharedPointer>
#include <QScopedPointer>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#include <opencv2/opencv.hpp>

#include "mainwindow.hpp"
#include "cvmatwidget.hpp"

typedef QSharedPointer<QAction> action_ptr;
typedef QWeakPointer<QMenu> menu_ptr;
typedef QScopedPointer<QTimer> timer_ptr;

template<>
struct ans::pimpl<cvcourse::mainwindow>::data
{
    std::vector<action_ptr> actions;
    cv::VideoCapture video;
    cvcourse::cvmatwidget canvas;
    timer_ptr timer;

    ~data()
    {
        video.release();
    }
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
    }

    void create_menus()
    {
        auto p = menuBar()->addMenu(tr("&Main"));
        boost::for_each(_data().actions, [&](action_ptr a){ p->addAction(a.data()); });
    }

    void initialize_layout()
    {
        //_data().canvas.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        setCentralWidget(&_data().canvas);
    }

    double delay()
    {
        BOOST_ASSERT(_data().video.isOpened());
        const double rate = _data().video.get(CV_CAP_PROP_FPS);
        const double d = 1000 / rate;
        //return boost::numeric_cast<int>(delay);
        return d;
    }

    void put_text(cv::Mat &img)
    {
        using namespace std;
        using namespace cv;

        // Use "y" to show that the baseLine is about
        string text = "Du Yu 11121043";
        int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontScale = 1;
        int thickness = 3;

        int baseline = 0;
        Size textSize = getTextSize(text, fontFace,
                                    fontScale, thickness, &baseline);
        baseline += thickness;

        // center the text
        Point textOrg((img.cols - textSize.width)/2,
                      (img.rows + textSize.height)/2);

        // then put the text itself
        putText(img, text, textOrg, fontFace, fontScale,
                Scalar(0, 0, 255), thickness, 8);
    }
};

cvcourse::mainwindow::mainwindow() : impl(use_default_ctor())
{
    _method(this).initialize_layout();
    _method(this).create_actions();
    _method(this).create_menus();
}

void cvcourse::mainwindow::open()
{
    QString name = QFileDialog::getOpenFileName(
        this,
        tr("Open video"),
        ".",
        tr("Video files (*.avi *.wmv *.mp4)")
        );

    bool ok = false;
    if (!name.isEmpty()) {
        _data().video.open(name.toAscii().data());
        ok = _data().video.isOpened();
    }

    //if (!ok) {
    //    QMessageBox::warning(this, tr("Open file"), tr("Open file failed."));
    //}
    if (ok) {
        _data().timer.reset(new QTimer(this));
        connect(_data().timer.data(), SIGNAL(timeout()), this, SLOT(update()));
        _data().timer->start(boost::numeric_cast<int>(_method(this).delay()));
    }
}

void cvcourse::mainwindow::update()
{
    BOOST_ASSERT(_data().video.isOpened());
    boost::timer tm;
    cv::Mat mat;
    if (_data().video.read(mat)) {
        _method(this).put_text(mat);
        _data().canvas.set_mat(mat);
        // include cv method time
        const double delay = _method(this).delay() - 1000 * tm.elapsed();
        _data().timer->start((std::max)(1, boost::numeric_cast<int>(delay)));
    } else {
        _data().timer.reset(nullptr);
    }
}
