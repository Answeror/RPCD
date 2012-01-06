/**
 *  @file
 *  @author answeror <answeror@gmail.com>
 *  @date 2012-01-06
 *  
 *  @section DESCRIPTION
 *  
 *  
 */

#include "yacvwidget.hpp"

#include <memory> // unique_ptr

#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_layout.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qfiledialog.h>
#include <qimagewriter.h>

#include <qwt_interval.h>
#include <qwt_scale_map.h>
#include <qimage.h>
#if QT_VERSION >= 0x040400
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#endif

#include <qwt_plot_rescaler.h>
#include <qwt_scale_div.h>

#include <opencv2/opencv.hpp>

#include <ans/alpha/pimpl.hpp>
#include <ans/alpha/pimpl_impl.hpp>

#include <iostream>
#include <boost/assert.hpp>

namespace
{
    class raster_data : public QwtMatrixRasterData
    {
    public:
        raster_data(const cv::Mat &mat)
        {
            cv::Mat3b rgb;
            cv::cvtColor(mat, rgb, CV_BGR2RGB);
            QVector<double> values;
            for (int y = rgb.size().height - 1; y > -1; --y)
            {
                for (int x = 0; x < rgb.size().width; ++x)
                {
                    auto val = rgb(y, x);
                    values += (int(val(0)) << 16) | (int(val(1)) << 8) | int(val(2));
                }
            }
            setValueMatrix(values, rgb.size().width);
            setInterval( Qt::XAxis, 
                QwtInterval( -0.5, rgb.size().width + 0.5, QwtInterval::ExcludeMaximum ) );
            setInterval( Qt::YAxis, 
                QwtInterval( -0.5, rgb.size().height + 0.5, QwtInterval::ExcludeMaximum ) );
            setInterval( Qt::ZAxis, QwtInterval(0.0, 0xffffffff) );

            //setResampleMode(QwtMatrixRasterData::BilinearInterpolation);
        }
    };

    class image_item : public QwtPlotRasterItem
    {
    public:
        image_item() : renderThreadCount(1) {}

        /*!
           \brief Render an image from data and color map.

           For each pixel of rect the value is mapped into a color.

          \param xMap X-Scale Map
          \param yMap Y-Scale Map
          \param area Requested area for the image in scale coordinates
          \param imageSize Size of the requested image

           \return A QImage::Format_Indexed8 or QImage::Format_ARGB32 depending
                   on the color map.

           \sa QwtRasterData::value(), QwtColorMap::rgb(),
               QwtColorMap::colorIndex()
        */
        QImage renderImage(
            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
            const QRectF &area, const QSize &imageSize ) const
        {
            if ( imageSize.isEmpty() || data == NULL)
            {
                return QImage();
            }

            const QwtInterval intensityRange = data->interval( Qt::ZAxis );
            if ( !intensityRange.isValid() )
                return QImage();

            QImage::Format format = QImage::Format_ARGB32;
            QImage image( imageSize, format );

            data->initRaster( area, image.size() );

        #if QT_VERSION >= 0x040400 && !defined(QT_NO_QFUTURE)
            uint numThreads = renderThreadCount;

            if ( numThreads <= 0 )
                numThreads = QThread::idealThreadCount();

            if ( numThreads <= 0 )
                numThreads = 1;

            const int numRows = imageSize.height() / numThreads;

            QList< QFuture<void> > futures;
            for ( uint i = 0; i < numThreads; i++ )
            {
                QRect tile( 0, i * numRows, image.width(), numRows );
                if ( i == numThreads - 1 )
                {
                    tile.setHeight( image.height() - i * numRows );
                    renderTile( xMap, yMap, tile, &image );
                }
                else
                {
                    futures += QtConcurrent::run(
                        this, &image_item::renderTile,
                        xMap, yMap, tile, &image );
                }
            }
            for ( int i = 0; i < futures.size(); i++ )
                futures[i].waitForFinished();

        #else // QT_VERSION < 0x040400
            const QRect tile( 0, 0, image.width(), image.height() );
            renderTile( xMap, yMap, tile, &image );
        #endif

            data->discardRaster();

            return image;
        }

        /*!
            \brief Render a tile of an image.

            Rendering in tiles can be used to composite an image in parallel
            threads.

            \param xMap X-Scale Map
            \param yMap Y-Scale Map
            \param tile Geometry of the tile in image coordinates
            \param image Image to be rendered
        */
        void renderTile(
            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
            const QRect &tile, QImage *image ) const
        {
            //const QwtInterval range = d_data->data->interval( Qt::ZAxis );
            //if ( !range.isValid() )
            //    return;
            if (!data) return;

            {
                for ( int y = tile.top(); y <= tile.bottom(); y++ )
                {
                    const double ty = yMap.invTransform( y );

                    QRgb *line = ( QRgb * )image->scanLine( y );
                    line += tile.left();

                    for ( int x = tile.left(); x <= tile.right(); x++ )
                    {
                        const double tx = xMap.invTransform( x );

                        auto val = static_cast<int>(data->value( tx, ty ));
                        *line++ = QColor((val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff).rgb();
                        //*line++ = val;
                    }
                }
            }
        }

        /*!
           Rendering an image from the raster data can often be done
           parallel on a multicore system.

           \param numThreads Number of threads to be used for rendering.
                             If numThreads is set to 0, the system specific
                             ideal thread count is used.

           The default thread count is 1 ( = no additional threads )

           \warning Rendering in multiple threads is only supported for Qt >= 4.4
           \sa renderThreadCount(), renderImage(), renderTile()
        */
        void setRenderThreadCount( uint numThreads )
        {
            renderThreadCount = numThreads;
        }

        /*!
          Set the data to be displayed

          \param data Spectrogram Data
          \sa data()
        */
        void setData( QwtRasterData *data )
        {
            if (this->data.get() != data )
            {
                this->data.reset(data);

                invalidateCache();
                itemChanged();
            }
        }

    private:
        uint renderThreadCount;
        std::unique_ptr<QwtRasterData> data;
    };
}

struct cvcourse::yacvwidget::impl
{
    cv::Mat mat;
    //QwtPlotSpectrogram *item;
    QwtPlotRescaler *rescaler;
};

namespace
{
    const ans::alpha::pimpl::use_default_ctor use_default_ctor;
}

cvcourse::yacvwidget::yacvwidget(QWidget *parent /*= nullptr*/) :
    base_type(parent),
    self(use_default_ctor)
{
    plotLayout()->setAlignCanvasToScales(true);

    //setAxisScale(QwtPlot::xBottom, 0, 1);
    //setAxisMaxMinor(QwtPlot::xBottom, 0);
    //setAxisScale(QwtPlot::yLeft, 0, 1);
    //setAxisMaxMinor(QwtPlot::yLeft, 0);

    auto magnifier = new QwtPlotMagnifier(canvas());
    magnifier->setAxisEnabled(QwtPlot::yRight, false);

    auto panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled( QwtPlot::yRight, false);

    self->rescaler = new QwtPlotRescaler(canvas());
    self->rescaler->setReferenceAxis(QwtPlot::xBottom);
    self->rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
    self->rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
    self->rescaler->setAspectRatio(QwtPlot::xTop, 0.0);

    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
    {
        self->rescaler->setIntervalHint(axis, QwtInterval(0.0, 1000.0));
    }

    //canvas()->setBorderRadius( 10 );
}

void cvcourse::yacvwidget::save()
{
    QString fileName = "rasterview.pdf";

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats =
        QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter("Images (");
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", fileName,
        filter.join(";;"), NULL, QFileDialog::DontConfirmOverwrite);
#endif
    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;
        renderer.renderDocument(this, fileName, QSizeF(300, 200), 85);
    }
}

bool cvcourse::yacvwidget::load()
{
    QString name = QFileDialog::getOpenFileName(
        this,
        tr("Open image"),
        ".",
        tr("Image files (*.png *.jpg *jpeg *bmp *gif)")
        );

    bool ok = false;
    if (!name.isEmpty()) {
        set(cv::imread(name.toAscii().data()));
        ok = !self->mat.empty();
    }
    return ok;
}

void cvcourse::yacvwidget::set(const cv::Mat &mat)
{
    self->mat = mat.clone();
    detachItems();
    //if (mat.empty()) {
    //    detachItems();
    //} else {
        // raster_data will be deleted internally
        auto item = new image_item;
        item->attach(this);
        item->setRenderThreadCount(0); // use system specific thread
        item->setData(new raster_data(self->mat));

        setAxisScale(QwtPlot::xBottom, 0, self->mat.size().width);
        setAxisMaxMinor(QwtPlot::xBottom, 0);
        setAxisScale(QwtPlot::yLeft, 0.0, self->mat.size().height);
        setAxisMaxMinor(QwtPlot::yLeft, 0);

        this->updateAxes();
    //}
}

const cv::Mat& cvcourse::yacvwidget::get() const
{
    return self->mat;
}

void cvcourse::yacvwidget::set_rescale_mode(int mode)
{
    bool doEnable = true;
    QString info;
    QRectF rectOfInterest;
    QwtPlotRescaler::ExpandingDirection direction = QwtPlotRescaler::ExpandUp;

    switch(mode)
    {
        case NONE:
        {
            doEnable = false;
            info = "All scales remain unchanged, when the plot is resized";
            break;
        }
        case FIXED:
        {
            self->rescaler->setRescalePolicy(QwtPlotRescaler::Fixed);
            info = "The scale of the bottom axis remains unchanged, "
                "when the plot is resized. All other scales are changed, "
                "so that a pixel on screen means the same distance for" 
                "all scales.";
            break;
        }
        case EXPANDING:
        {
            self->rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
            info = "The scales of all axis are shrinked/expanded, when "
                "resizing the plot, keeping the distance that is represented "
                "by one pixel.";
            //d_rescaleInfo->setText("Expanding");
            break;
        }
        case FITTING:
        {
            self->rescaler->setRescalePolicy(QwtPlotRescaler::Fitting);
            const QwtInterval xIntv = 
                self->rescaler->intervalHint(QwtPlot::xBottom);
            const QwtInterval yIntv = 
                self->rescaler->intervalHint(QwtPlot::yLeft);

            //rectOfInterest = QRectF( xIntv.minValue(), yIntv.minValue(),
            //    xIntv.width(), yIntv.width());
            direction = QwtPlotRescaler::ExpandBoth;

            info = "Fitting";
            break;
        }
    }

    //d_plot->setRectOfInterest(rectOfInterest);

    //d_rescaleInfo->setText(info);
    self->rescaler->setEnabled(doEnable);
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
    {
        self->rescaler->setExpandingDirection(direction);
    }

    if ( doEnable ) {
        self->rescaler->rescale();
    } else {
        replot();
    }
}
