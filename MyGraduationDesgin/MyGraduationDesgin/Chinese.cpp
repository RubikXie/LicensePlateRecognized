//
//  Chinese.cpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/5/2.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#include "Chinese.hpp"

string province[40] = {"桂", "沪", "闵", "粤", "鄂",
                       "鲁", "闵", "辽", "苏", "津",
                       "陕", "苏", "豫", "沪", "贵",
                       "沪", "京", "闵", "沪", "苏",
                       "川", "黑", "鄂", "鄂", "沪",
                       "苏", "鄂", "粤", "皖", "湘",
                       "粤", "鄂", "云", "苏", "渝",
                       "豫", "湘", "鄂", "湘", "湘",
};

string getChinese(Mat img)
{
    threshold(img, img, 0, 255, THRESH_OTSU | THRESH_BINARY);
    vector<float> v;
    float _max = -2;
    int index = -1;
    for (int i = 0; i < 40; i++)
    {
        Mat res(img.rows + 1, img.cols + 1, CV_32FC1);
        string path = "/Users/xieshiyong/c++/NEURAL_NET/train_set/chinese/" + to_string(i) + ".jpg";
        //cout << path << endl;
        Mat CHN_template = imread(path);
        cvtColor(CHN_template, CHN_template, CV_BGR2GRAY);
        threshold(CHN_template, CHN_template, 0, 255, THRESH_BINARY | THRESH_OTSU);
        
        matchTemplate(img, CHN_template, res, CV_TM_CCORR_NORMED, Mat());
        v.push_back(res.at<float>(0, 0));
    }
    
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i] > _max)
        {
            index = i;
            _max = v[i];
        }
    }
    return province[index];
}
