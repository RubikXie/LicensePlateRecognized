//
//  PlateLocation.cpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/4/22.
//  Copyright © 2018年 xieshiyong. All rights reserved.


#include "platelocation.h"


///////////////////////////////////////////////////////////////////
//非核心函数
///////////////////////////////////////////////////////////////////
PlateLocation* PlateLocation::location = NULL;

PlateLocation* PlateLocation::instance()
{
    if (location == NULL)
    {
        location = new PlateLocation();
    }
    return location;
}

PlateLocation::~PlateLocation()
{
    delete location;
}

void PlateLocation::debug(bool flag)
{
    _debug_ = flag;
}

bool PlateLocation::debug()
{
    return _debug_;
}

void PlateLocation::maxAngle(double angle)
{
    max_angle = angle;
}

double PlateLocation::maxAngle()
{
    return max_angle;
}

PlateLocation::PlateLocation()
{
    this->gaussian_size = Size(3, 3);
    this->morph_colsed_size = Size(17, 3);
    this->sobel_x_scale = 1.0;
    this->sobel_y_scale = 0.0;
    min_area = 500;
    max_area = 35000;
    min_length_scale = 1.8;
    max_length_scale = 5.3;
    this->max_angle = 45;
    this->_debug_ = true;
    addBackgroundColor(min_blue, max_blue);
    addBackgroundColor(min_yellow, max_yellow);
    min_plate_num = 1;
}

int PlateLocation::minPlateNum()
{
    return min_plate_num;
}
void PlateLocation::minPlateNum(int cnt)
{
    min_plate_num  = cnt;
}

int PlateLocation::addBackgroundColor(Vec3b minHsv, Vec3b maxHsv)
{
    background_color.push_back(minHsv);
    background_color.push_back(maxHsv);
    return (int)background_color.size() / 2 + 1;
}

int PlateLocation::clearColorInfo()
{
    background_color.clear();
    font_color.clear();
    return 0;
}

int PlateLocation::addFontColor(Vec3b minHsv, Vec3b maxHsv)
{
    font_color.push_back(minHsv);
    font_color.push_back(maxHsv);
    return (int)font_color.size() / 2 + 1;
}


int PlateLocation::addColorInfo(pair<Vec3b, Vec3b> background, pair<Vec3b, Vec3b> font)
{
    int cnt = addBackgroundColor(background.first, background.second);
    addFontColor(font.first, font.second);
    return cnt;
}

//HSV是否合法
bool PlateLocation::isValidColor(uchar h, uchar s, uchar v)
{
    for (int i = 0; i < (int)background_color.size(); i += 2)
    {
        Vec3b min_hsv = background_color[i];
        Vec3b max_hsv = background_color[i + 2];

        if (h < min_hsv[0] || s < min_hsv[1] || v < min_hsv[2] || h > max_hsv[0] || s > max_hsv[1] || v > max_hsv[2])
        {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////
//核心函数
///////////////////////////////////////////////////////////////////

//通过颜色预处理
bool PlateLocation::colorPretreatment(Mat& src, Mat& dst, int color_index)
{
    if (background_color.size() == 0)
    {
        return false;
    }

    vector<Mat> vecMat;
    maxAngle(60);
    max_area = 90000;
    Mat src_hsv, temp;
    cvtColor(src, src_hsv, CV_BGR2HSV);
    myshow("temp", src_hsv);
    Vec3b min_hsv = background_color[2 * color_index + 0],  max_hsv = background_color[2 * color_index + 1];
    //cout << "mis_hsv" << min_hsv << endl;
    //cout << "max_hsv" << max_hsv << endl;

    inRange(src_hsv, Scalar(min_hsv[0], min_hsv[1], min_hsv[2]), Scalar(max_hsv[0], max_hsv[1], max_hsv[2]), temp);

    //    Mat ele = getStructuringElement(MORPH_RECT, Size(3, 3));
    //    morphologyEx(temp, temp, MORPH_CLOSE, ele);

    myshow("temp", temp);
    BFSFilter(temp);

    temp.copyTo(dst);

    return true;
}

//大致过滤掉一些噪声
void PlateLocation::BFSFilter(Mat& img)
{
    int dir[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    Mat tag = Mat::zeros(img.rows, img.cols, img.type());
    Mat temp = Mat(img.size(), img.type(), Scalar(0));

    myshow("BFSFIlter img", img);

    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            queue<Point> q;
            vector<Point> v;
            Point temp;
            int maxr = -1, maxb = -1, minu = img.rows + 1, minl = img.cols + 1;

            if (img.at<uchar>(i, j) == 255 && tag.at<uchar>(i, j) == 0)
            {
                temp.x = i;
                temp.y = j;
                v.push_back(temp);
                q.push(temp);

                tag.at<uchar>(i, j) = 1;
                //temp.x is rows
                //temp.y is cols
                maxr = max(maxr, temp.y);
                minl = min(minl, temp.y);
                maxb = max(maxb, temp.x);
                minu = min(minu, temp.x);
                while (!q.empty())
                {
                    Point top = q.front();
                    q.pop();

                    for (int k = 0; k < 4; k++)
                    {
                        int tempi = top.x + dir[k][0];
                        int tempj = top.y + dir[k][1];

                        if (tempi >= 0 && tempi < img.rows && tempj >= 0 && tempj < img.cols && img.at<uchar>(tempi, tempj) == 255 && tag.at<uchar>(tempi, tempj) == 0)
                        {
                            temp.x = tempi;
                            temp.y = tempj;
                            maxr = max(maxr, temp.y);
                            minl = min(minl, temp.y);
                            maxb = max(maxb, temp.x);
                            minu = min(minu, temp.x);
                            tag.at<uchar>(tempi, tempj) = 1;
                            v.push_back(temp);
                            q.push(temp);
                        }
                    }
                }

                int width = maxr - minl + 1;
                int height = maxb - minu + 1;

                if (v.size() < min_area ||  v.size() > max_area || width < 50 || height < 10)
                {
                    if (_debug_)
                    {
                        //printf("size = %d\n", (int)v.size());
                        //printf("width = %d\n", width);
                        //printf("height = %d\n", height);
                    }

                    for (int k = 0; k < (int)v.size(); k++)
                    {
                        img.at<uchar>(v[k].x, v[k].y) = 0;
                    }

                    if (_debug_)
                    {
                        //myshow("BFSFilter debug", img);
                    }
                }
            }
        }
    }
}

bool PlateLocation::rectPretreatment(Mat&src, Mat& dst)
{
    Mat temp;
    Mat sobel_x, sobel_y;

    maxAngle(30);
    max_area = 35000;
    GaussianBlur(src, temp, Size(5, 5), 0.0, 0.0, BORDER_DEFAULT);
    myshow("temp", temp);
    cvtColor(temp, temp, CV_RGB2GRAY);
    myshow("temp", temp);
    Sobel(temp, sobel_x, CV_16S, 1, 0, 3, 1, 0);
    convertScaleAbs(sobel_x, sobel_x);
    Sobel(temp, sobel_y, CV_16S, 0, 1, 3, 1, 0);
    convertScaleAbs(sobel_y, sobel_y);

    addWeighted(sobel_x, 1, sobel_y, 0, 0, temp);

    myshow("temp", temp);
    threshold(temp, temp, 0, 255, THRESH_OTSU | THRESH_BINARY);
    myshow("temp", temp);
    Mat ele = getStructuringElement(MORPH_RECT, Size(17, 3));
    morphologyEx(temp, temp, MORPH_CLOSE, ele);
    myshow("temp", temp);
    BFSFilter(temp);

    temp.copyTo(dst);
    return true;
}

void PlateLocation::myshow(string winname, Mat& img)
{
    if (_debug_)
    {
        namedWindow(winname, WINDOW_AUTOSIZE);
        imshow(winname, img);
        waitKey();
        destroyWindow(winname);
    }
}

vector<vector<Point> > PlateLocation::findROI(Mat& img)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    if (_debug_ & 0)
    {
        Mat debugMat;
        img.copyTo(debugMat);

        for (int i = 0; i < (int)contours.size(); i++)
        {
            drawContours(debugMat, contours, i, Scalar(128), 3, LINE_8);
            myshow("findROI debug", debugMat);
        }
    }

    return contours;
}

vector<RotatedRect> PlateLocation::FilterRIO(Mat& img, vector<vector<Point> > contours)
{
    vector<RotatedRect> rect;
    RotatedRect temp;
    Mat debugMat;
    img.copyTo(debugMat);
    for (int i = 0; i < (int)contours.size(); i++)
    {
        temp = minAreaRect(contours[i]);

        if (!isValidROI(img, temp))
        {
            if (_debug_)
            {
                Point2f points[4];
                temp.points(points);
                //cout << "[Invalid]  ";
                //cout << "angle = " << temp.angle;
                //cout << "  width = " << temp.size.width;
                //cout << "  height = " << temp.size.height;
                //cout << "  area = " << temp.size.height * temp.size.width << endl;

                for (int j = 0; j < 4; j++)
                {
                    line(debugMat, points[j], points[(j + 1) % 4], Scalar(128), 3, LINE_8);
                }
                myshow("invalid rect", debugMat);
            }
        }
        else
        {
            rect.push_back(temp);
        }
    }

    if (_debug_)
    {
        img.copyTo(debugMat);

        Point2f points[4];
        for (int i = 0; i < (int)rect.size(); i++)
        {
            RotatedRect temp = rect[i];
//            cout << "[valid]  ";
//            cout << "angle = " << temp.angle;
//            cout << "  width = " << temp.size.width;
//            cout << "  height = " << temp.size.height;
//            cout << "  area = " << temp.size.height * temp.size.width << endl;
//            cout << "point";
            for (int i = 0; i < 4; i++)
            {
                //cout << points[i] << " ";
            }
            //cout << endl;
            rect[i].points(points);
            for (int j = 0; j < 4; j++)
            {
                line(debugMat, points[j], points[(j + 1) % 4], Scalar(128), 3, LINE_8);
            }
            myshow("valid rect", debugMat);
        }
    }

    return rect;
}

bool PlateLocation::isValidROI(Mat& img, RotatedRect rect)
{
    Point2f points[4];
    rect.points(points);

    for (int i = 0; i < 4; i++)
    {
        if (points[i].x < 0 || points[i].x > img.cols || points[i].y < 0 || points[i].y > img.rows)
        {
            return false;
        }
    }

    float r = rect.size.width / rect.size.height;
    if (r < 1)
    {
        rect.angle += 90;
        swap(rect.size.width, rect.size.height);
    }

    double width = rect.size.width;
    double height = rect.size.height;
    double length_scale = width / height;
    double area = width * height;

    if (_debug_)
    {
        //printf("width = %g height = %g angle = %g length_scale = %g area = %g\n", width, height, rect.angle, length_scale, area);
    }

    //角度过滤
    if (abs(rect.angle) > max_angle)
    {
        return false;
    }

    //大小过滤
    if (length_scale < min_length_scale || length_scale > max_length_scale)
    {
        return false;
    }

    if (area < min_area || area > max_area)
    {
        return false;
    }

    return true;
}

vector<Mat> PlateLocation::getRotatedROI(Mat& src, vector<RotatedRect> rect)
{
    vector<Mat> roi;
    for (int i = 0; i < (int)rect.size(); i++)
    {
        Mat temp;
        Mat img;
        src.copyTo(img);
        if (_debug_)
        {
//            cout << "width = " << rect[i].size.width;
//            cout << " height = " << rect[i].size.height;
//            cout << " angle = " << rect[i].angle << endl;
        }

        if (rect[i].size.width < rect[i].size.height)
        {
            swap(rect[i].size.width, rect[i].size.height);
            rect[i].angle += 90;
        }

        Mat M = getRotationMatrix2D(rect[i].center, rect[i].angle, 1);
        warpAffine(img, img, M, img.size());

        getRectSubPix(img, rect[i].size, rect[i].center, temp);
        resize(temp, temp, Size(136, 36));
        roi.push_back(temp);

    }

    return roi;
}

vector<Mat> PlateLocation::getPlates(vector<Mat>& roi)
{
    vector<Mat> plates;
    int size_ = (int)background_color.size() / 2 + 1;

    for (int i = 0; i < (int)roi.size(); i++)
    {
        Mat temp[size_];
        int cnt_max = -1;
        int color_index = -1;
        Mat hsv;
        cvtColor(roi[i], hsv, CV_BGR2HSV);
        for (int j = 0; j < (int)background_color.size(); j += 2)
        {
            Vec3b min_hsv = background_color[j],  max_hsv = background_color[j + 1];
//            cout << "mis_hsv" << min_hsv << endl;
//            cout << "max_hsv" << max_hsv << endl;
            temp[j].create(hsv.size(), CV_8UC(1));

//            for (int r = 0; r < hsv.rows; r++)
//            {
//                for (int c = 0; c < hsv.cols; c++)
//                {
//                    Vec3b pixel = hsv.at<Vec3b>(r, c);
//                    if (pixel[0] >= min_hsv[0] && pixel[0] <= max_hsv[0])
//                    {
//                        if (pixel[1] >= min_hsv[1] && pixel[1] <= max_hsv[1] && pixel[2] >= min_hsv[2] && pixel[2] <= max_hsv[2])
//                        {
//                            if (j == 0)
//                            {
//                                temp[j].at<uchar>(r, c) = 255;
//                            }
//                            else
//                            {
//                                if (pixel[1] + pixel[2] >= (min_hsv[1] + max_hsv[1]-20))
//                                {
//                                    temp[j].at<uchar>(r, c) = 255;
//                                }
//                                else
//                                {
//                                    temp[j].at<uchar>(r, c) = 0;
//                                }

//                            }
//                        }
//                        else
//                        {
//                            temp[j].at<uchar>(r, c) = 0;
//                        }
//                    }
//                    else
//                    {
//                        temp[j].at<uchar>(r, c) = 0;
//                    }
//                }
//            }
            inRange(hsv, Scalar(min_hsv[0], min_hsv[1], min_hsv[2]), Scalar(max_hsv[0], max_hsv[1], max_hsv[2]), temp[j] );
            int cnt = 0;

            for (int r  = 0; r < temp[j].rows; r++)
            {
                for (int c = 0; c < temp[j].cols; c++)
                {
                    if (temp[j].at<uchar>(r, c) == 255)
                    {
                        cnt++;
                    }
                }
            }

            if (cnt > cnt_max)
            {
                cnt_max = cnt;
                color_index = j;
            }
        }

        if (isValidRC(temp[color_index]))
        {
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;
            Mat ele = getStructuringElement(MORPH_RECT, Size(3,3));
            morphologyEx(temp[color_index], temp[color_index], MORPH_CLOSE, ele);
            findContours(temp[color_index], contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

            RotatedRect max_rect;
            double max_area = -1;
            for (int i = 0; i < (int)contours.size(); i++)
            {
                RotatedRect rr = minAreaRect(contours[i]);
                if (rr.size.area() > max_area)
                {
                    max_area = rr.size.area();
                    max_rect = rr;
                }
            }
            if (max_rect.size.width < max_rect.size.height)
            {
                swap(max_rect.size.width, max_rect.size.height);
                max_rect.angle += 90;
            }
            Mat rotated_roi;
            Mat M = getRotationMatrix2D(max_rect.center, max_rect.angle, 1);
            warpAffine(roi[i], rotated_roi, M, roi[i].size());
            getRectSubPix(roi[i], max_rect.size, max_rect.center, roi[i]);
            resize(roi[i], roi[i], Size(136,36));

            if (_debug_)
            {
                namedWindow("roi", WINDOW_AUTOSIZE);
                //namedWindow("plate", WINDOW_AUTOSIZE);

                imshow("roi", roi[i]);
                //imshow("plate", temp[color_index]);
                waitKey();
                destroyWindow("roi");
                //destroyWindow("plate");

            }
        }
        else
        {
            if (_debug_)
            {
                namedWindow("invalid roi", WINDOW_AUTOSIZE);
                //namedWindow("invalid plate", WINDOW_AUTOSIZE);

                imshow("invalid roi", roi[i]);
                //imshow("invalid plate", temp[color_index]);
                waitKey();
                destroyWindow("invalid roi");
                //destroyWindow("invalid plate");
            }

            roi.erase(roi.begin() + i);
            i--;
        }

    }

    return plates;
}

bool PlateLocation::isValidRC(Mat& img)
{
    Mat img1;
    int col_cnt = img.cols * 0.6;
    int row_cnt = img.rows * 0.6;
    int pixel = 0;
    Mat ele = getStructuringElement(MORPH_RECT, Size(3,3));
    morphologyEx(img, img1, MORPH_CLOSE, ele);
    myshow("is Valid RC?", img1);
    for (int i = 0; i < img1.rows; i++)
    {
        for (int j = 0; j < img1.cols; j++)
        {
            if (img1.at<uchar>(i, j) == 255)
            {
                pixel++;
            }
        }
    }
    //cout << "pixel = " << pixel << endl;

    if (pixel < img1.cols * img1.rows * 0.30)
    {
        return false;
    }

    int max_col = -1;
    for (int i = 0; i < img1.rows; i++)
    {
        int temp = 0;
        for (int j = 0; j < img1.cols; j++)
        {
            if (img1.at<uchar>(i, j) == 255)
            {
                temp++;
            }
            else
            {
                if (temp > max_col)
                {
                    max_col = temp;
                    if (max_col > col_cnt)
                    {
                        return true;
                    }
                }
                temp = 0;
            }
        }

        if (temp > max_col)
        {
            max_col = temp;
            if (max_col > col_cnt)
            {
                return true;
            }
        }
    }

    int max_row = -1;
    for (int j = 0; j < img1.cols; j++)
    {
        int temp = 0;
        for (int i = 0; i < img1.rows; i++)
        {
            if (img1.at<uchar>(i, j) == 255)
            {
                temp++;
            }
            else
            {
                if (temp > max_row)
                {
                    max_row = temp;
                    if (max_row > row_cnt)
                    {
                        return true;
                    }
                }
                temp = 0;
            }
        }

        if (temp > max_row)
        {
            max_row = temp;
            if (max_row > row_cnt)
            {
                return true;
            }
        }

    }
    return false;
}

vector<Mat> PlateLocation::findPlateEdge(Mat src)
{
    Mat dst;
    //矩形预处理
    location->rectPretreatment(src, dst);
    //location->colorPretreatment(src, dst);
    //寻找边缘
    vector<vector<Point> > contours = location->findROI(dst);
    //过滤边缘
    location->maxAngle(30);
    vector<RotatedRect> rect = location->FilterRIO(dst, contours);
    //获取边缘
    vector<Mat> roi = location->getRotatedROI(src, rect);
    location->getPlates(roi);

    return roi;
}


vector<Mat> PlateLocation::findPlateColor(Mat src)
{
    Mat dst;
    vector<Mat> ans;
    //矩形预处理
    //location->rectPretreatment(src, dst);
    int types = (int)background_color.size() / 2;

    for (int i = 0; i < types; i++)
    {
        location->colorPretreatment(src, dst, i);
        //寻找边缘
        vector<vector<Point> > contours = location->findROI(dst);
        //过滤边缘
        location->maxAngle(60);
        //获取边缘
        vector<RotatedRect> rect = location->FilterRIO(dst, contours);
        vector<Mat> roi = location->getRotatedROI(src, rect);
        location->getPlates(roi);

        for (int k = 0; k < (int)roi.size(); k++)
        {
            ans.push_back(roi[k]);

        }
    }

    return ans;
}









