//
//  OCR.cpp
//  MyGraduationDesgin
//
//  Created by xieshiyong on 2018/5/2.
//  Copyright © 2018年 xieshiyong. All rights reserved.
//

#include "MyOCR.hpp"
char ch[35] = {"23456789ABCDEFGH1JKLMN0PQRSTUVWXYZ"};

Ptr<ANN_MLP> ann;

Mat getHist(Mat& img)
{
    int col[12] = {0}, row[22] = {0};
    Mat hist(1, 12 + 22, CV_32F, Scalar(0));
    
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            if (img.at<uchar>(i, j) == 255)
            {
                col[j]++;
                row[i]++;
            }
        }
    }
    
    for (int i = 0; i < 12; i++)
    {
        hist.at<float>(0, i) = col[i];
    }
    
    for (int i = 0; i < 22; i++)
    {
        hist.at<float>(0, i + 12) = row[i];
    }
    
    return hist;
    
}

Mat getFeatures(Mat img, Size size)
{
    Mat feature(1, img.cols + img.rows + size.area(), CV_32F, Scalar(0));
    int cnt = 0;
    Mat hist = getHist(img);
    Mat small;
    
    threshold(img, img, 0, 255, THRESH_OTSU | THRESH_BINARY);
    resize(img, small, size);
    
    for (int i = 0; i < hist.cols; i++)
    {
        feature.at<float>(0, cnt++) = hist.at<uchar>(0, i);
    }
    
    for (int i = 0; i < small.rows; i++)
    {
        for (int j = 0; j < small.cols; j++)
        {
            feature.at<float>(0, cnt++) = (float)small.at<uchar>(i, j);
        }
    }
    
    return feature;
}

void train()
{
    Mat trainingData, classes;
    vector<int> trainingLabels;
    string path = "/Users/xieshiyong/c++/NEURAL_NET/train_set/samples/";
    for (int index = 2; index <= 9; index++)
    {
        for (int i = 0; i <= 500; i++)
        {
            string basePath = path + to_string(index) + "/ts" + to_string(i) + ".jpg";
            //cout << basePath << endl;
            Mat temp = imread(basePath);
            
            if (temp.data == NULL)
            {
                break;
            }
            //myshow("img", temp);
            cvtColor(temp, temp, CV_BGR2GRAY);
            Mat feature = getFeatures(temp, Size(6, 11));
            trainingData.push_back(feature);
            trainingLabels.push_back(index - 2);
        }
    }
    
    for (int index = 0; index < 26; index++)
    {
        for (int i = 0; i <= 500; i++)
        {
            string basePath = path + (char)(index + 'A') + "/ts" + to_string(i) + ".jpg";
            //cout << basePath << endl;
            Mat temp = imread(basePath);
            
            if (temp.data == NULL)
            {
                break;
            }
            cvtColor(temp, temp, CV_BGR2GRAY);
            Mat feature = getFeatures(temp, Size(6, 11));
            trainingData.push_back(feature);
            trainingLabels.push_back(index + 8);
        }
    }
    
    trainingData.convertTo(trainingData, CV_32F);
    Mat(trainingLabels).copyTo(classes);
    
    
    FileStorage fs("/Users/xieshiyong/c++/NEURAL_NET/OCR.xml", FileStorage::WRITE);
    fs << "trainingData" << trainingData;
    fs << "classes" << classes;
    fs.release();
    
}


void classificationANN()
{
    FileStorage fs;
    fs.open("/Users/xieshiyong/c++/NEURAL_NET/OCR.xml", FileStorage::READ);
    Mat TrainingData;
    Mat classes;
    fs["trainingData"] >> TrainingData;
    fs["classes"] >> classes;
    int nlayers = 10;
    // step1. 生成训练数据
    Mat trainClasses;
    trainClasses.create(TrainingData.rows, 34, CV_32FC1);
    for (int i = 0; i < trainClasses.rows; i++) {
        for (int j = 0; j < trainClasses.cols; j++) {
            if (j == classes.at<int>(i))
                trainClasses.at<float>(i, j) = 1;
            else
                trainClasses.at<float>(i, j) = 0;
        }
    }
    Ptr<TrainData> trainingData = TrainData::create(TrainingData, ROW_SAMPLE, trainClasses);
    
    // step2. 创建分类器
    Mat layers(1, 3, CV_32SC1);
    layers.at<int>(0) = TrainingData.cols;
    layers.at<int>(1) = nlayers;
    layers.at<int>(2) = 34;
    
    ann = ANN_MLP::create();
    ann->setLayerSizes(layers); // 设置层数
    ann->setActivationFunction(ANN_MLP::SIGMOID_SYM, 1, 1); // 设置激励函数
    
    // step3. 训练
    ann->train(trainingData);
}

int predictANN(Mat& f)
{
    // step4. 预测
    // 处理输入的特征Mat f
    Mat output(1, 34, CV_32FC1);
    ann->predict(f, output); // 开始预测
    
    
    Point maxLoc;
    double maxVal;
    // output为一个向量，向量的每一个元素反映了输入样本属于每个类别的概率
    minMaxLoc(output, 0, &maxVal, 0, &maxLoc); // 找到最大概率
    
    // 返回字符在strCharacters[]数组中的索引
    return maxLoc.x;
}

string getString(vector<Mat> v, int flag)
{
    string res = "";
    if (v.size() == 0) return "error";
    
    if (flag == 0)
    {
        if (v.size() != 7) return "error";
        
        res += getChinese(v[0]);
        for (int i = 1; i < v.size(); i++)
        {
            Mat feature = getFeatures(v[i], Size(6, 11));
            int index = predictANN(feature);
            if (i == 1 && ch[index] == '4')
            {
                res += 'A';
            }
            else if (i == 1 && ch[index] == '8')
            {
                res += 'B';
            }
            else
            {
                res += ch[index];
            }
            
        }
    }
    else
    {
        if (v.size() != 2) return "error";
        res += getChinese(v[0]);
        tesseract::TessBaseAPI tess;
        tess.Init(NULL, "plates", tesseract::OEM_DEFAULT);
        tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
        tess.SetImage((uchar*)v[1].data, v[1].cols, v[1].rows, 1, v[1].cols);

        //Get the text;
        char* tesseract_out = tess.GetUTF8Text();
        for (int i = 0; tesseract_out[i] != '\0'; i++)
        {
            res += tesseract_out[i];
        }
    }
    res[9] ='\0';
    return res;
}
