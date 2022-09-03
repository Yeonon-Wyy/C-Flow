/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-27 22:24:33
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 17:41:42
 */
#pragma once

#include <opencv2/imgproc/types_c.h>

#include <mutex>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#include "../../src/core/utils/time_util.hpp"
#include "../../src/core/utils/log/log.hpp"
#include "FrameRequest.hpp"

using namespace std;
using namespace cv;

class FrameRequest;

class dnnfacedetect
{
private:
    CascadeClassifier faceCascade;
    CascadeClassifier eyes_Cascade;
    bool              m_isInit = false;
    std::mutex        m_mutex;
    //构造函数 传入模型文件
    dnnfacedetect();

public:
    static dnnfacedetect *getInstance()
    {
        static dnnfacedetect fd;
        return &fd;
    }
    void config();
    ~dnnfacedetect();

    //初始化DNN网络
    bool initdnnNet();

    //人脸检测
    bool detect(std::shared_ptr<FrameRequest> request);
    bool detectFix(std::shared_ptr<FrameRequest> request);
};

dnnfacedetect::dnnfacedetect()
{
    char filepath[256];
    getcwd(filepath, sizeof(filepath));
    CFLOW_LOGD("filePath: {0}", filepath);
    //定义模型文件
    string faceBindr = (string)filepath + "/haarcascade_frontalface_alt2.xml";
    string eyeBindry = (string)filepath + "/haarcascade_eye_tree_eyeglasses.xml";

    if (!faceCascade.load(faceBindr))
    {
        CFLOW_LOGE("人脸检测级联分类器没找到！！");
    }
    if (!eyes_Cascade.load(eyeBindry))
    {
        CFLOW_LOGE("人脸检测级联分类器没找到！！");
    }

    CFLOW_LOGD("dnnfaceDected construct, this {0}", fmt::ptr(this));
}

void dnnfacedetect::config()
{
    if (!initdnnNet())
    {
        CFLOW_LOGD("init faild");
        return;
    }
    m_isInit = true;
    CFLOW_LOGD("init success");
}

dnnfacedetect::~dnnfacedetect() {}

//初始化dnnnet
bool dnnfacedetect::initdnnNet() { return true; }

//人脸检测
bool dnnfacedetect::detect(std::shared_ptr<FrameRequest> request)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    TRACE_FUNC_ID_START(__FUNCTION__, request->ID());

    Mat  imgGray;
    auto img = *(request->getFrame());
    cvtColor(img, imgGray, CV_BGR2GRAY);
    equalizeHist(imgGray, imgGray);  //直方图均匀化
    vector<Rect> faces, eyes;
    faceCascade.detectMultiScale(imgGray, faces, 1.2, 5, 0, Size(30, 30));
    for (auto b : faces)
    {
        CFLOW_LOGD("face roi [{0},{1},{2},{3}]", b.x, b.y, b.width, b.height);
    }
    if (faces.size() > 0)
    {
        for (size_t i = 0; i < faces.size(); i++)
        {
            rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 1, 8);
            cout << faces[i] << endl;
            //将人脸从灰度图中抠出来
            Mat face_ = imgGray(faces[i]);
            eyes_Cascade.detectMultiScale(face_, eyes, 1.2, 2, 0, Size(30, 30));
            for (size_t j = 0; j < eyes.size(); j++)
            {
                Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
                int   radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
                circle(img, eye_center, radius, Scalar(65, 105, 255), 4, 8, 0);
            }
        }
    }

    return true;
}

bool dnnfacedetect::detectFix(std::shared_ptr<FrameRequest> request)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    auto frame = *(request->getFrame());
    //计算矩形
    int xLeftBottom = static_cast<int>(160);
    int yLeftBottom = static_cast<int>(120);
    int xRightTop   = static_cast<int>(160 + 320);
    int yRightTop   = static_cast<int>(120 + 240);
    //生成矩形
    Rect rect((int)xLeftBottom, (int)yLeftBottom, (int)(xRightTop - xLeftBottom), (int)(yRightTop - yLeftBottom));

    //在原图上用红框画出矩形
    rectangle(frame, rect, Scalar(0, 0, 255));
    std::this_thread::sleep_until(cflow::utils::TimeUtil::awake_time(33));
    return true;
}