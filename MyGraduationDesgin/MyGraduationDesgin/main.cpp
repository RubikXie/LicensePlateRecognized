
#include <iostream>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <cstdio>
#include "PlateLocation.hpp"
#include "SVM.hpp"
#include "segment.hpp"
#include "MyOCR.hpp"
using namespace std;
using namespace cv;

string basePath = "/Users/xieshiyong/c++/MyGraduationDesgin/Image/p";
string writePath = "/Users/xieshiyong/c++/MyGraduationDesgin/output/out";
string platePath = "/Users/xieshiyong/c++/MyGraduationDesgin/plate/p";
string no_platePath = "/Users/xieshiyong/c++/MyGraduationDesgin/not_plate/p";
bool isDebug = true;
int cnt = 0;
int _save_ = 0   ;
int recognize_funs = 0;
void save(int flag, int cnt, Mat& img)
{
    if (_save_ == 0) return ;
    string path;
    if (flag == 1)
    {
        path = platePath;
    }
    else
    {
        path = no_platePath;
    }
    
    path += to_string(cnt) + ".jpg";
    
    imwrite(path, img);
}

typedef vector<Mat> (PlateLocation::*Local)(Mat);

int main()
{
    int cnt = 0;
    int location_num = 0;
    int saveNUm[2];
    PlateLocation *location = PlateLocation::instance();
    cout << " SVM is training..." << endl;
    classification();
    cout << "SVM finished" << endl;
    
    cout << "Neural Net is training..." << endl;
    classificationANN();
    cout << "Neural Net finished" << endl;
    
    Local func[2];
    func[0] = &PlateLocation::findPlateEdge;
    func[1] = &PlateLocation::findPlateColor;
    //location->debug(false);
    for (int i = 0 ; i <= 658; i++)
    {
        cout << "---------------" << i << "------------------" << endl;
        int func_index = 0;
        string index = to_string(i);
        string path = basePath + index;
        Mat src = imread(path + ".jpg");
        Mat dst;
        if (src.data == NULL)
        {
            src = imread(path + ".jpeg");
            if (src.data == NULL)
            {
                continue;
            }
        }

        namedWindow("src");
        imshow("src", src);
        waitKey();
        destroyWindow("src");
        int sum_plates = 0;
        for (int k = 0; k < 2; k++)
        {
            vector<Mat> roi;
            if (sum_plates < location->minPlateNum())
            {
                roi = (location->*func[func_index % 2])(src);
                func_index++;
                for (int s = 0;  s < roi.size(); s++)
                {
                    Mat dst = processROI(roi[s]);
                    Mat feature = getFeature(dst);
                    int flag = SVMPredict(feature);
                    myshow(to_string(flag), roi[s]);
                    save(flag, saveNUm[flag]++, roi[k]);
                    
                    if (flag == 1)
                    {
                        vector<Mat> plate_ch = segmented(roi[s], recognize_funs);
                        string res =  getString(plate_ch, recognize_funs);
                        sum_plates++;
                        if (k == 0 && res == "error")
                        {
                            sum_plates--;
                        }
                        else if (res == "error" && k == 1)
                        {
                            cout << "error" << endl;
                        }
                        else if (res != "error")
                        {
                            cout << res << endl;
                            namedWindow("plate");
                            imshow("plate", roi[s]);
                            waitKey();
                            destroyWindow("plate");

                        }
                    }
                    else
                    {
                        roi.erase(roi.begin() + s);
                        s--;
                    }
                    location_num += roi.size();
                }
            }
        }
        
        
        if (location_num >= location->minPlateNum())
        {
            cnt++;
        }
    }
    
    cout << "location successed : " << cnt << endl;
    return 0;
}
