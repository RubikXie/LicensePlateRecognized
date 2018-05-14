//
//  segment.hpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/5/2.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#ifndef segment_hpp
#define segment_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int IsBlue(Vec3b p);
void filterRow(Mat& thresh, int row, int l, int r);
int PlateType(Mat& hsv);
int lineNumber(Mat& thresh, int row);
void filterline(Mat& thresh, int row);
Mat process(Mat t, vector<int>& h);
bool isCharter(Rect rect, vector<int> h);
vector<Rect> updateVec(vector<Rect> v);
vector<Mat> segmented(Mat src, int flag = 0);

#endif /* segment_hpp */
