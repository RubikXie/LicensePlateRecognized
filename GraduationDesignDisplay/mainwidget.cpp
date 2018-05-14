#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QFontDatabase>
#include <QIODevice>
#include <QDebug>
#include <QFile>
#include <QFont>


vector<Mat> rois;
vector<string> recognize_res;
typedef vector<Mat> (PlateLocation::*Local)(Mat);
int plateIndex = 0;
int srcIndex = 0;
int recognize_funs = 0;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    basePath = "/Users/xieshiyong/c++/MyGraduationDesgin/recognize/p";
    //basePath = "/Users/xieshiyong/c++/MyGraduationDesgin/Image/p";
    location = PlateLocation::instance();

    cout << " SVM is training..." << endl;
    classification();
    cout << "SVM finished" << endl;

    cout << "Neural Net is training..." << endl;
    classificationANN();
    cout << "Neural Net finished" << endl;

    ui->setupUi(this);
    pageInitial(0);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::pageInitial(int index)
{
    srcIndex = index;
    plateIndex = 0;
    ui->spinBox->setValue(index);
    rois.clear();
    recognize_res.clear();
    plateRecognize(index, rois, recognize_res);
    string path = basePath + to_string(index) + ".jpg";
    Mat img = imread(path);


    if (img.cols >= img.rows)
    {
        int newcols = 400;
        int newrows = 400.0 / img.cols * img.rows;
        cv::resize(img, img, Size(newcols, newrows));
    }
    else
    {
        int newcols = 400.0 / img.rows * img.cols;
        int newrows = 400;
        cv::resize(img, img, Size(newcols, newrows));
    }

    ui->label->setAlignment(Qt::AlignCenter);
    QPixmap pix = mat2pix(img);
    ui->label->setPixmap(pix);

    if (rois.size() > 0)
    {
        ui->label_3->setPixmap(mat2pix(rois[plateIndex]));
        ui->label_3->setAlignment(Qt::AlignCenter);
        string num_text = to_string(plateIndex + 1) + "/" + to_string((int)rois.size());
        ui->label_5->setText(num_text.data());
        ui->lineEdit->setText(recognize_res[0].data());
    }
    else
    {
        ui->lineEdit->setText("没有定位到车牌");
        ui->label_5->setText("0/0");
    }

}

void MainWidget::plateRecognize(int index, vector<Mat>& rois, vector<string>& recognization)
{
    Local func[2];
    func[0] = &PlateLocation::findPlateEdge;
    func[1] = &PlateLocation::findPlateColor;
    location->debug(false);

    Mat src = imread(basePath + to_string(index) + ".jpg");
    if (src.data == NULL)
    {
        src = imread(basePath + to_string(index) + ".jpeg");
        if (src.data == NULL)
        {
            return;
        }
    }

    int sum_plates = 0;
    int func_index = 0;

    for (int k = 0; k < 2; k++)
    {
        vector<Mat> roi;
        if (sum_plates < location->minPlateNum())
        {
            roi = (location->*func[func_index % 2])(src);
            func_index++;
            for (int s = 0;  s < (int)roi.size(); s++)
            {
                Mat dst = processROI(roi[s]);
                Mat feature = getFeature(dst);
                int flag = SVMPredict(feature);

                //save(flag, saveNUm[flag]++, roi[k]);

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
                        //cout << "error" << endl;
                        rois.push_back(roi[s]);
                        recognization.push_back("error");
                    }
                    else if (res != "error")
                    {
                        //cout << res << endl;
                        rois.push_back(roi[s]);
                        recognization.push_back(res);

                    }
                }
                else
                {
                    roi.erase(roi.begin() + s);
                    s--;
                }
            }
        }
    }
}

QPixmap MainWidget::mat2pix(Mat& img)
{
    QImage qimg;

    if (img.channels() == 1)
    {
        qimg =QImage((const unsigned char*)(img.data),
                            img.cols, img.rows,
                            img.cols * img.channels(),
                            QImage::Format_Indexed8);
        qimg.bits();

    }
    else if (img.channels() == 3)
    {
        Mat temp;
        cvtColor(img, temp, CV_BGR2RGB);
        qimg =QImage((const unsigned char*)(temp.data),
                            temp.cols, temp.rows,
                            temp.cols * temp.channels(),
                            QImage::Format_RGB888);
        qimg.bits();
    }

    return QPixmap::fromImage(qimg);
}



void MainWidget::on_pushButton_clicked()
{
    if (plateIndex > 0)
    {
        plateIndex--;
        string str = to_string(plateIndex + 1) + "/" + to_string((int)rois.size());
        ui->label_5->setText(str.data());
        ui->label_3->setPixmap(mat2pix(rois[plateIndex]));
        ui->lineEdit->setText(recognize_res[plateIndex].data());
    }
}

void MainWidget::on_pushButton_2_clicked()
{
    if (plateIndex < (int)rois.size() - 1)
    {
        plateIndex++;
        string str = to_string(plateIndex + 1) + "/" + to_string((int)rois.size());
        ui->label_5->setText(str.data());
        ui->label_3->setPixmap(mat2pix(rois[plateIndex]));
        ui->lineEdit->setText(recognize_res[plateIndex].data());
    }
}

void MainWidget::on_pushButton_3_clicked()
{
    if (srcIndex > 0)
    {
        pageInitial(srcIndex - 1);
    }
}

void MainWidget::on_pushButton_4_clicked()
{
    if (srcIndex < 200)
    {
       pageInitial(srcIndex + 1);
    }
}

void MainWidget::on_pushButton_5_clicked()
{
    int index = ui->spinBox->value();
    pageInitial(index);
}
