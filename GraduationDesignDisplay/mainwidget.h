#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <cstdio>
#include "platelocation.h"
#include "svm.h"
#include "segment.h"
#include "myocr.h"

using namespace std;
using namespace cv;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

private:
    string basePath;
    PlateLocation *location;
    Ui::MainWidget *ui;
    void plateRecognize(int index, vector<Mat>& rois, vector<string>& recognization);
    void pageInitial(int index);
    QPixmap mat2pix(Mat& img);
};

#endif // MAINWIDGET_H
