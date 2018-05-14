#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace ml;

char ch[35] = {"23456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

Ptr<ANN_MLP> ann;

int _debug = 1;
void myshow(string name, Mat& img, int flag = 0)
{
    if (_debug || flag)
    {
        namedWindow(name);
        imshow(name, img);
        waitKey();
        destroyWindow(name);
    }
}

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
            cout << basePath << endl;
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
            cout << basePath << endl;
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


void classificationANN(Mat TrainingData, Mat classes, int nlayers){
    
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

int main(int argc, const char * argv[])
{
    //Mat img = imread("/Users/xieshiyong/Desktop/character/ch33.jpg");

    //myshow("img", img);
    //train();
    FileStorage fs;
    fs.open("/Users/xieshiyong/c++/NEURAL_NET/OCR.xml", FileStorage::READ);
    Mat TrainingData;
    Mat Classes;
    fs["trainingData"] >> TrainingData;
    fs["classes"] >> Classes;
    classificationANN(TrainingData, Classes, 10);

    int cnt = 0, ans = 0;
    for (int i = 1; i < 2; i++)
    {
        for (int j = 0; j < 500; j++)
        {
            string path = "/Users/xieshiyong/Desktop/charSamples/1/";
            Mat img = imread(path + to_string(j) + ".jpg");
            if (img.data == NULL)
            {
                break;
            }
            resize(img, img, Size(12, 22));
            cvtColor(img, img, CV_BGR2GRAY);
            Mat feature = getFeatures(img, Size(6, 11));

            //waitKey();

            cnt++;
            int index = predictANN(feature);
            cout << ch[index] << "  " << ch[i] << endl;
            if (ch[index] == 'I')
            {
                ans++;
            }
        }
    }

    cout << "总计：" << cnt << endl;
    cout <<  "正确：" << ans << endl;
    cout << "正确率:" << 1.0 * ans / cnt << endl;
    return 0;
}
