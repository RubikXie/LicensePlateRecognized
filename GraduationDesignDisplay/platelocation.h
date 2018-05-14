//
//  PlateLocation.hpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/4/22.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#ifndef plateLocation_hpp
#define plateLocation_hpp

#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#define BLUE 0
#define YELLOW 1

using namespace  std;
using namespace cv;

const Vec3b min_blue(100, 50, 50), max_blue(150, 255, 255);
const Vec3b min_yellow(15, 40, 40), max_yellow(55, 255, 255);

class PlateLocation
{
    public :
    //创建示例
    static PlateLocation* instance();
    ~PlateLocation();
    //添加车牌颜色信息
    int addColorInfo(pair<Vec3b, Vec3b> background, pair<Vec3b, Vec3b> font);
    int minPlateNum();
    void minPlateNum(int cnt);
    //设置调试模式
    void debug(bool flag);
    bool debug();
    //设置最大误差角度
    void maxAngle(double angle);
    double maxAngle();
    //清楚颜色信息

    vector<Mat> findPlateEdge(Mat src);
    vector<Mat> findPlateColor(Mat src);

    int clearColorInfo();

    //添加背景、字体颜色
    int addBackgroundColor(Vec3b minHsv, Vec3b maxHsv);
    int addFontColor(Vec3b minHsv, Vec3b maxHsv);

    protected :
    //被保护的构造函数
    PlateLocation();
    private :
    bool isValidColor(uchar h, uchar s, uchar v);
    void BFSFilter(Mat& img);
    void myshow(string winname, Mat& img);
    bool _debug_;
    bool isValidROI(Mat& img, RotatedRect rect);
    bool isValidRC(Mat& img);
    Size gaussian_size, morph_colsed_size;      //高斯模糊，闭操作卷积大小
    double sobel_x_scale, sobel_y_scale;        //sobel算子x、y方向比例
    double min_area, max_area;                  //车牌的大小范围
    double min_length_scale, max_length_scale;  //车牌长宽比例范围
    vector<Vec3b> background_color;             //背景颜色 min max min max
    vector<Vec3b> font_color;                   //字体颜色 同理
    double max_angle;
    static PlateLocation *location;
    int min_plate_num;


    //通过颜色预处理
    bool colorPretreatment(Mat& src, Mat& dst, int color_index = 0);
    //通过矩形预处理
    bool rectPretreatment(Mat&src, Mat& dst);
    //寻找轮廓
    vector<vector<Point> > findROI(Mat& img);
    //过滤轮廓
    vector<RotatedRect> FilterRIO(Mat& img, vector<vector<Point> > contours);
    //旋转矩形
    vector<Mat> getRotatedROI(Mat& src, vector<RotatedRect> rect);
    vector<Mat> getPlates(vector<Mat>& roi);
};


#endif /* plateLocation_hpp */
