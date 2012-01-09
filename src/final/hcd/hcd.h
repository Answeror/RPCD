/****************************************************************************
**
** For Copyright & Licensing information, see COPYRIGHT in project root
**
****************************************************************************/

#include <QImage>
#include <QVector>

#include <vector>

#include <opencv2/opencv.hpp>

typedef QVector<unsigned int> IntArray;
typedef QVector<IntArray>     Image;

class HoughCircleDetector
{
  public: /* class */
  
    HoughCircleDetector() {}
   ~HoughCircleDetector() {}
  
  public: /* methods */

    QImage detect(const QImage &source, unsigned int min_r, unsigned int max_r);

    std::vector<cv::Vec4f> yadetect(
        //const QImage &source,
        const cv::Mat1b &source,
        double cover_radio = 0.5, // covered by vote
        int kernel_radius = 1,
        unsigned int min_r = 0, unsigned int max_r = 0);
  
  private: /* methods */
  
    void accum_circle(Image &image, const QPoint &position, unsigned int radius);
    void accum_pixel(Image &image, const QPoint &position);
  
    void draw_circle(QImage &image, const QPoint &position, unsigned int radius, const QColor &color);
    void draw_pixel(QImage &image, const QPoint &position, const QColor &color);
    
    QImage edges(const QImage &source);
};

