
//
//  SVM.cpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/4/28.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#include "SVM.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace ml;

Ptr<SVM> svm;

int isBlue(Vec3b p)
{
    int ok = 1;
    const Vec3b min_blue(100, 50, 50), max_blue(150, 255, 255);
    
    for (int i = 0; i < 3; i++)
    {
        if (p[i] < min_blue[i] || p[i] > max_blue[i])
        {
            return 0;
        }
    }
    
    return ok;
}

int plateType(Mat& hsv)
{
    int cnt = 0;
    for (int r = 0; r < hsv.rows; r++)
    {
        for (int c = 0; c < hsv.cols; c++)
        {
            Vec3b pixel = hsv.at<Vec3b>(r, c);
            cnt += isBlue(pixel);
        }
    }
    if (cnt >= (hsv.rows * hsv.cols * 0.6))
    {
        return 1;
    }
    return 0;
}

Mat getFeature(Mat& img)
{
    Mat feature;
    feature.create(1, 172, CV_32FC1);
    int col = 0;;
    for (int i = 0; i < img.rows; i++)
    {
        int cnt = 0;
        for (int j = 0; j <img.cols; j++)
        {
            if (img.at<uchar>(i, j) == 255)
            {
                cnt++;
            }
        }
        feature.at<float>(0, col++) = cnt;
    }
    
    for (int j = 0; j < img.cols; j++)
    {
        int cnt = 0;
        for (int i = 0; i < img.rows; i++)
        {
            if (img.at<uchar>(i, j) == 255)
            {
                cnt++;
            }
        }
        feature.at<float>(0, col++) = cnt;
    }
    
    return feature;
}


/* 基于SVM的图像分类 */
bool classification()
{
    FileStorage fs;
    fs.open("/Users/xieshiyong/c++/MyGraduationDesgin/SVM.xml", FileStorage::READ);
    Mat trainingDataMat;
    Mat classesMat;
    fs["TrainingData"] >> trainingDataMat;
    fs["classes"] >> classesMat;
    Ptr<TrainData> trainingData = TrainData::create(trainingDataMat, ROW_SAMPLE, classesMat);
    
    
    SVM::KernelTypes kernel_type = SVM::RBF;
    svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setKernel(kernel_type);
    svm->trainAuto(trainingData);
    
    return 1;
}

void myshow(string name, Mat& img)
{
    namedWindow(name, WINDOW_AUTOSIZE);
    imshow(name, img);
    waitKey();
    destroyWindow(name);
}

int SVMPredict(Mat feature)
{
    return svm->predict(feature);
}

Mat processROI(Mat img)
{
    Mat hsv;
    cvtColor(img, hsv, CV_BGR2HSV);
    
    int type = plateType(hsv);
    
    cvtColor(img, img, CV_BGR2GRAY);
    if (type == 1)
    {
        threshold(img, img, 0, 255, THRESH_BINARY | THRESH_OTSU);
    }
    else
    {
        threshold(img, img, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    }
    return img;
}
