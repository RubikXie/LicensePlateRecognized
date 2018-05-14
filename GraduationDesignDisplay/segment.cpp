//
//  segment.cpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/5/2.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#include "segment.h"

using namespace std;
using namespace cv;

int IsBlue(Vec3b p)
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

void filterRow(Mat& thresh, int row, int l, int r)
{
    uchar *p = thresh.ptr(row);

    for (int i = l; i <= r; ++i)
    {
        p[i] = 0;
    }
}

int PlateType(Mat& hsv)
{
    int cnt = 0;
    for (int r = 0; r < hsv.rows; r++)
    {
        for (int c = 0; c < hsv.cols; c++)
        {
            Vec3b pixel = hsv.at<Vec3b>(r, c);
            if (IsBlue(pixel))
            {
                cnt++;
            }

        }
    }
    if (cnt >= (hsv.rows * hsv.cols * 0.6))
    {
        return 1;
    }
    return 0;
}

int lineNumber(Mat& thresh, int row)
{
    int cnt = 0;
    int start = -1;
    uchar *p = thresh.ptr(row);
    for (int i = 0; i < thresh.cols; i++)
    {
        if (p[i] == 255 && start == -1)
        {
            start = i;
        }

        if (p[i] == 0 && start != -1)
        {
            cnt++;
            start = -1;
        }
    }

    if (start != -1)
    {
        cnt++;
    }
    return cnt;
}

void filterline(Mat& thresh, int row)
{
    int len = 0;
    uchar *p = thresh.ptr(row);
    for (int i = 0; i < thresh.cols; i++)
    {
        if (p[i] == 255)
        {
            len++;
        }
        else
        {
            if (len >= 18)
            {
                int j = 0;
                while (j != len)
                {
                    p[i - j] = 0;
                    j++;
                }
            }
            len = 0;
        }
    }

    if (len >= 16)
    {
        int j = 0;
        while (j != len)
        {
            p[135 - j] = 0;
            j++;
        }
    }

}

Mat process(Mat t, vector<int>& h)
{
    Mat img, hsv, gray, thresh;
    t.copyTo(img);

    GaussianBlur(img, img, Size(3, 3), 0);
    cvtColor(img, hsv, CV_BGR2HSV);
    cvtColor(img, gray, CV_BGR2GRAY);

    int type = PlateType(hsv);

    if (type == 1)
    {
        threshold(gray, thresh, 0, 255, THRESH_OTSU | THRESH_BINARY);
    }
    else
    {
        threshold(gray, thresh, 0, 255, THRESH_OTSU | THRESH_BINARY_INV);
    }

    int cnt = 0;
    for (int i = 0; i < thresh.rows; i++)
    {
        for (int j = 0; j < thresh.cols; j++)
        {
            if (thresh.at<uchar>(i, j) == 255)
            {
                cnt++;
            }
        }
    }
    //cout << "cnt = " << cnt << endl;
    if (cnt > 2700)
    {
        for (int i = 0; i < thresh.rows; i++)
        {
            for (int j = 0; j < thresh.cols; j++)
            {
                thresh.at<uchar>(i, j) = 255 - thresh.at<uchar>(i, j);
            }
        }

    }
    //    namedWindow("thresh");
    //    imshow("thresh", thresh);
    //    waitKey();
    //    destroyWindow("thresh");

    //myshow("thresh", thresh);

    //去初每行除铆钉、边缘
    for (int i = 0; i < thresh.rows; i++)
    {
        int cnt = lineNumber(thresh, i);

        if (cnt < 7)
        {
            filterRow(thresh, i, 0, thresh.cols - 1);
        }
    }
    //    namedWindow("thresh");
    //    imshow("thresh", thresh);
    //    waitKey();
    //    destroyWindow("thresh");
    //myshow("thresh *", thresh);

    //去除较长的连线
    for (int i = 0; i < 3; i++)
    {
        filterline(thresh, i);
    }

    for (int i = 35; i >= 33; i--)
    {
        filterline(thresh, i);
    }

    //myshow("thresh --", thresh);
    //垂直投影
    for (int j = 0; j < thresh.cols; j++)
    {
        int cnt = 0;
        for (int i = 0; i < thresh.rows; i++)
        {
            if (thresh.at<uchar>(i, j) == 255)
            {
                cnt++;
            }
        }
        h.push_back(cnt);
    }

    for (int i = 0; i < thresh.cols; i++)
    {
        int count1 = 0, count2 = 0;
        for (int j = 0; j <= 4; j++)
        {
            if (thresh.at<uchar>(j, i) == 255)
            {
                count1++;
            }
            if (thresh.at<uchar>(35 - j, i) == 255)
            {
                count2++;
            }
        }


        for (int j = 0; j <= 4; j++)
        {
            if (thresh.at<uchar>(j, i) == 255 && count1 <= 3 && h[i] < 5)
            {
                thresh.at<uchar>(j, i) = 0;
            }
            if (thresh.at<uchar>(35 - j, i) == 255 && count2 <= 3 && h[i] < 5)
            {
                thresh.at<uchar>(35 - j, i) = 0;
            }
        }


    }

    //myshow("thresh |", thresh);

    return thresh;
}

bool isCharter(Rect rect, vector<int> h)
{
    Rect tx;
    //椒盐噪声
    if (rect.area() < 30)
    {
        return false;
    }

    //高度不够
    if (rect.height < 15)
    {
        return false;
    }

    //1或者车牌银边的讨论
    if ((double)rect.height / rect.width >= 3)
    {
        Point tl = rect.tl(), br = rect.br();
        int width = rect.height / 4;
        //width--;
        if (tl.x < 20) return false;
        if (tl.x - width < 0) return false;
        //if (br.x > 134) return false;

        for (int i = tl.x - (h[tl.x] != 0), k = width; i >= 0 && k > 0; i--, k--)
        {
            if (h[i] > 5)
            {
                return false;
            }
        }

        for (int i = br.x + (h[br.x] != 0), k = width; i <= 132 && k > 0; i++, k--)
        {
            if (h[i] > 5)
            {
                return false;
            }
        }
    }

    if (rect.tl().x < 20)
    {
        if (rect.width < 8)
        {
            return false;
        }
    }

    return true;
}

bool cmp(Rect x, Rect y)
{
    return x.tl().x < y.tl().x;
}

vector<Rect> updateVec(vector<Rect> v)
{
    vector<Rect> ans;
    int y_min = 100, y_max = -1;
    int all_height = 0;
    int avg_width = 0, sum = 0, num = 0;
    for (int i = 0; i < (int)v.size(); i++)
    {
        y_min = min(v[i].tl().y, y_min);
        y_max = max(v[i].br().y, y_max);

        if (v[i].width > 8 && v[i].width <= 18)
        {
            sum += v[i].width;
            num++;
        }
    }
    if (num == 0)
    {
        ans.clear();
        return ans;
    }
    all_height = y_max - y_min;
    avg_width = sum / num + (sum % num != 0);
    int gap = avg_width / 4;
    //int bigger_gap = all_height * 0.75;

    if (v[0].tl().x > avg_width + gap)
    {
        if (v[0].tl().x < (2 * avg_width + 3 * gap))
        {
            Rect r(v[0].tl().x - gap - avg_width - 1, y_min, avg_width + 2, all_height);
            v.insert(v.begin() + 0, r);
        }
        else
        {
            return ans;
        }
    }

    while (v.size() > 7)
    {
        v.pop_back();
    }


    //刚好七个字符
    if (v.size() == 7)
    {
        for (int i = 0; i < (int)v.size(); i++)
        {
            Rect temp;
            if (v[i].width > 8)
            {
                /*temp = Rect(v[i].tl().x - 1, y_min, v[i].width + 2, all_height);*/
                int x = (avg_width - v[i].width) / 2;
                temp = Rect(v[i].tl().x - x - 1, y_min, avg_width + 2, all_height);
            }
            else
            {
                int x = (avg_width - v[i].width) / 2;
                temp = Rect(v[i].tl().x - x - 1, y_min, avg_width + 2, all_height);
            }
            ans.push_back(temp);
        }
        return ans;
    }
    //前两个字符连在一起
    if (v[0].width > 20 && v.size() == 6)
    {
        Rect r1(v[0].tl().x - 1, y_min, avg_width + 2, all_height);
        Rect r2(v[0].tl().x + 1 + avg_width, y_min, avg_width + 2, all_height);
        ans.push_back(r1);
        ans.push_back(r2);

        for (int i = 1; i < (int)v.size(); i++)
        {
            Rect temp;
            if (v[i].width > 8)
            {
                /*temp = Rect(v[i].tl().x - 1, y_min, v[i].width + 2, all_height);*/
                int x = (avg_width - v[i].width) / 2;
                temp = Rect(v[i].tl().x - x -1, y_min, avg_width + 2, all_height);
            }
            else
            {
                int x = (avg_width - v[i].width) / 2;
                temp = Rect(v[i].tl().x - x - 1, y_min, avg_width + 2, all_height);
            }
            ans.push_back(temp);

        }
        return ans;
    }

    ans.clear();

    for (int i = 0; i < (int)v.size(); i++)
    {
        if (v[i].width > 22)
        {
            Rect r1(v[i].tl().x, y_min, v[i].width / 2, all_height);
            Rect r2(v[i].br().x - v[i].width / 2, y_min, v[i].width / 2, all_height);
            ans.push_back(r1);
            ans.push_back(r2);
        }
        else if (v[i].width < 8)
        {
            int x = (avg_width - v[i].width) / 2;
            Rect temp = Rect(v[i].tl().x - x - 1, y_min, avg_width + 2, all_height);
            ans.push_back(temp);
        }
        else
        {
            Rect temp;
            int x = (avg_width - v[i].width) / 2;
            temp = Rect(v[i].tl().x - x - 1, y_min, avg_width + 2, all_height);
            ans.push_back(temp);
        }
    }

    if (ans.size() == 7)
    {
        return ans;
    }

    v.clear();
    for (int i = 0; i < (int)ans.size(); i++)
    {
        v.push_back(ans[i]);
    }
    ans.clear();
    //缺字符，计算。
    int _begin = 0;
    for (int i = 0; i < (int)v.size();i++)
    {
        int temp = abs(v[i].tl().x - _begin);

        if (temp >= (avg_width + gap - 2))
        {
            if (temp > (1.5 * avg_width + gap))
            {
                if (ans.size() == 1)
                {
                    Rect standrad = ans[0];
                    Rect r(standrad.br().x + gap - 1, y_min, avg_width + 2, all_height);
                    ans.push_back(r);
                    _begin = ans[ans.size() - 1].br().x + 3 * gap - 1;
                }

                temp = v[i].tl().x - _begin;
                vector<Rect> rect_vec;
                int count = 1;
                while (temp > (avg_width + gap))
                {
                    Rect r(v[i].tl().x - count * (gap + avg_width + 1) - (count - 1), y_min, avg_width + 2, all_height);
                    rect_vec.push_back(r);
                    temp -= (gap + avg_width + 1);
                }

                for (int k = (int)rect_vec.size() - 1; k >= 0; k--)
                {
                    ans.push_back(rect_vec[k]);
                }

                ans.push_back(v[i]);
                _begin = v[i].br().x;
                continue;
            }

            if (ans.size() != 0 && ans.size() != 2)
            {
                Rect standrad = ans[ans.size() - 1];
                Rect r(standrad.br().x + gap - 1, y_min, avg_width, all_height);
                ans.push_back(r);
                ans.push_back(v[i]);
                _begin = v[i].br().x;
            }
            else if (ans.size() == 0)
            {
                Rect r(v[i].tl().x - gap - avg_width - 1, y_min, avg_width + 2, all_height);
                ans.push_back(r);
                ans.push_back(v[i]);
            }
            else
            {
                ans.push_back(v[i]);
            }
            _begin = v[i].br().x;

        }
        else
        {
            if (v[i].width > 22)
            {
                Rect r1(v[i].tl().x, y_min, v[i].width / 2, all_height);
                Rect r2(v[i].br().x - v[i].width / 2, y_min, v[i].width / 2, all_height);
                ans.push_back(r1);
                ans.push_back(r2);

            }
            //ans.push_back(v[i]);
            _begin = v[i].br().x;

        }
    }

    while (ans.size() > 7)
    {
        ans.pop_back();
    }

    return ans;
}

vector<Mat> segmented(Mat src, int flag)
{
    vector<int> h;
    vector<Mat> ch;
    Mat dst = process(src, h);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(dst, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector<Rect> rect;
    for (int i = 0; i < (int)contours.size(); i++)
    {
        Rect temp = boundingRect(contours[i]);
        if (isCharter(temp, h))
        {
            rect.push_back(temp);
        }
    }


    sort(rect.begin(), rect.end(), cmp);

    while (rect.size() > 7)
    {
        rect.pop_back();
    }

    vector<Rect> vec_rect = updateVec(rect);


    Mat l(42, 12 * 15, CV_8UC(1), Scalar(0, 0, 0));
    for (int i = 0; i < (int)vec_rect.size(); i++)
    {
        Mat temp;
        Rect rr = vec_rect[i];
        if (rr.br().x > 135)
        {
            rr.width = 135 - rr.tl().x;
        }
        if (rr.tl().x < 0)
        {
            rr.x = 0;
        }
        while (rr.tl().y < 0)
        {
            rr.y = 0;
        }
        while (rr.br().y > 135)
        {
            rr.y = 135;
        }
        if (i != 0)
            temp = dst(rr);
        else
            temp = src(rr);

        if (temp.type() == CV_8UC(3))
        {
            cvtColor(temp, temp, CV_BGR2GRAY);
            threshold(temp, temp, 0, 255, THRESH_BINARY | THRESH_OTSU);
        }

        resize(temp, temp, Size(12, 22));
        if (flag == 0)
        {
            ch.push_back(temp);
        }
        else
        {
            if (0 == i)
            {
                ch.push_back(temp);
            }
            else
            {
                Rect rrr = Rect((2 * i + 1) * 12, 10, temp.cols, temp.rows);
                temp.copyTo(l(rrr));
            }
        }
    }

    if (flag == 1)
    {
        ch.push_back(l);
    }
    return ch;
}

