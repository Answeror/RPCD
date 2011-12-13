#include <QApplication>

#include "mainwindow.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    cvcourse::mainwindow win;
    win.show();
    return app.exec();
}

//#include <opencv2/opencv.hpp>
//
//int main()
//{
//    using namespace cv;
//    auto img = imread("castle.jpg");
//    Mat tmp;
//    cvtColor(img, tmp, CV_BGR2RGB);
//    cvtColor(tmp, img, CV_RGB2BGR);
//    namedWindow("image with grain", CV_WINDOW_AUTOSIZE);
//    imshow("image with grain", img);
//    waitKey();
//    return 0;
//}