/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-27 22:24:33
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 21:18:13
 */
#pragma once

#include <opencv2/imgproc/types_c.h>

#include <mutex>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#include "../../src/core/utils/time_util.hpp"
#include "../../src/core/utils/log/log.hpp"
#include "FrameRequest.hpp"

using namespace cv;

class FrameRequest;

class dnnfacedetect
{
private:
    CascadeClassifier faceCascade;
    CascadeClassifier eyes_Cascade;
    bool m_isInit = false;
    std::mutex m_mutex;
    //构造函数 传入模型文件
    dnnfacedetect();

    Rect translateROI(Rect& originROI, float scaleRatio);

public:
    static dnnfacedetect* getInstance()
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
};

dnnfacedetect::dnnfacedetect()
{
    char filepath[256];
    getcwd(filepath, sizeof(filepath));
    CFLOW_LOGD("filePath: {0}", filepath);
    //定义模型文件
    std::string faceBindr =
        (std::string)filepath + "/haarcascade_frontalface_alt2.xml";
    std::string eyeBindry =
        (std::string)filepath + "/haarcascade_eye_tree_eyeglasses.xml";

    if (!faceCascade.load(faceBindr))
    {
        CFLOW_LOGE("haarcascade_frontalface_alt2.xml can't found");
    }
    if (!eyes_Cascade.load(eyeBindry))
    {
        CFLOW_LOGE("haarcascade_eye_tree_eyeglasses can't found");
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
    TRACE_FUNC_ID_START(__FUNCTION__, request->ID());
    std::unique_lock<std::mutex> lk(m_mutex);

    auto get240PFrame = [](auto&& img) -> Mat {
        int originWidth = img.cols;
        int originHeight = img.rows;
        float dsRatio = (float)originHeight / 240.0f;
        int newWidth = originWidth / dsRatio;
        int newHeight = 240;
        Mat fdImg;
        cv::resize(img, fdImg, Size(newWidth, newHeight), dsRatio, dsRatio,
                   INTER_LINEAR);
        return fdImg;
    };
    Mat imgGray;
    Mat img = *(request->getFrame());
    float dsRatio = (float)img.rows / 240.0f;
    Mat fdImg = get240PFrame(img);
    CFLOW_LOGD("[debug] fdImg w/h {0}/{1}", fdImg.cols, fdImg.rows);

    cvtColor(fdImg, imgGray, CV_BGR2GRAY);
    equalizeHist(imgGray, imgGray); //直方图均匀化
    std::vector<Rect> faces, eyes;
    {
        faceCascade.detectMultiScale(imgGray, faces, 1.2, 5, 0, Size(30, 30));
    }
    for (auto face : faces)
    {
        CFLOW_LOGD("face roi [{0},{1},{2},{3}]", face.x, face.y, face.width,
                   face.height);
    }
    if (faces.size() > 0)
    {
        for (size_t i = 0; i < faces.size(); i++)
        {
            auto newROI = translateROI(faces[i], dsRatio);
            request->addFaceRect(newROI);
        }
    }

    return true;
}

Rect dnnfacedetect::translateROI(Rect& originROI, float scaleRatio)
{
    Rect newROI;
    newROI.x = scaleRatio * originROI.x;
    newROI.y = scaleRatio * originROI.y;
    newROI.width = scaleRatio * originROI.width;
    newROI.height = scaleRatio * originROI.height;
    return newROI;
}