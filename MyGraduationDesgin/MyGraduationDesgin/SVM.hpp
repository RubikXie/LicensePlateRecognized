//
//  SVM.hpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/4/28.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#ifndef SVM_hpp
#define SVM_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//训练数据
bool classification();

//第一步处理ROI
Mat processROI(Mat img);
int isBlue(Vec3b p);
int plateType(Mat& hsv);

//获取特征
Mat getFeature(Mat& img);
//预测
int SVMPredict(Mat feature);

void myshow(string name, Mat& img);

#endif /* SVM_hpp */
