//
//  OCR.hpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/5/2.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#ifndef OCR_hpp
#define OCR_hpp

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/text.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <tesseract/baseapi.h>
#include "Chinese.hpp"
using namespace std;
using namespace cv;
using namespace ml;
using namespace cv::text;

Mat getHist(Mat& img);
Mat getFeatures(Mat img, Size size);
void train();
void classificationANN();
int predictANN(Mat& f);
string getString(vector<Mat> v, int flag = 0);
#endif /* OCR_hpp */
