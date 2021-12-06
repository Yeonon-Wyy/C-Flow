/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-27 22:24:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-06 22:35:52
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <mutex>

#include "FrameRequest.hpp"
#include "../../src/core/utils/time_util.hpp"

using namespace std;
using namespace cv;

class FrameRequest;

class dnnfacedetect
{
private:
  string _modelbinary, _modeldesc;
  dnn::Net _net;
  bool m_isInit = false;
  std::mutex m_mutex;
  //构造函数 传入模型文件
  dnnfacedetect();
  // dnnfacedetect(string modelBinary, string modelDesc);


public:
  static dnnfacedetect* getInstance() {
    static dnnfacedetect fd;
    return &fd;
  }

  void config();

  ~dnnfacedetect();
  //置信阈值
  float confidenceThreshold;
  double inScaleFactor;
  size_t inWidth;
  size_t inHeight;
  Scalar meanVal;

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
	VTF_LOGD("filePath: {0}", filepath);
	//定义模型文件
	string ModelBinary = (string)filepath + "/opencv_face_detector_uint8.pb";
	string ModelDesc = (string)filepath + "/opencv_face_detector.pbtxt";

  _modelbinary = ModelBinary;
  _modeldesc = ModelDesc;
  VTF_LOGD("dnnfaceDected construct, this {0}", fmt::ptr(this));
  //初始化置信阈值
  confidenceThreshold = 0.6;
  inScaleFactor = 0.5;
  inWidth = 300;
  inHeight = 300;
  meanVal = Scalar(104.0, 177.0, 123.0);
}



void dnnfacedetect::config()
{
  if (!initdnnNet())
  {
    VTF_LOGD("init faild");
    return;
  }
  m_isInit = true;
  VTF_LOGD("init success");
}

dnnfacedetect::~dnnfacedetect()
{
  _net.~Net();
}

//初始化dnnnet
bool dnnfacedetect::initdnnNet()
{
  VTF_LOGD("_modelbinary {0}, _modeldesc {1}", _modelbinary, _modeldesc);
  _net = dnn::readNetFromTensorflow(_modelbinary, _modeldesc);
  _net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
  _net.setPreferableTarget(dnn::DNN_TARGET_CPU);

  return !_net.empty();
}

//人脸检测
bool dnnfacedetect::detect(std::shared_ptr<FrameRequest> request)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  VTF_LOGD("faceDected inScaleFactor = {0} this {1}", inScaleFactor, fmt::ptr(this));
  auto start = std::chrono::system_clock::now();
  Mat tmpsrc = *(request->getFrame());
  // 修改通道数
  if (tmpsrc.channels() == 4)
    cvtColor(tmpsrc, tmpsrc, COLOR_BGRA2BGR);
  // 输入数据调整
  Mat detection;
  {
    Mat inputBlob = dnn::blobFromImage(tmpsrc, inScaleFactor,
                                      Size(inWidth, inHeight), meanVal, false, false);
    //人脸检测
    _net.setInput(inputBlob, "data");
    detection = _net.forward("detection_out");
  }



  Mat detectionMat(detection.size[2], detection.size[3],
                  CV_32F, detection.ptr<float>());
  VTF_LOGD("faceDected dected {0} face, inScaleFactor = {1} this {2}", detectionMat.rows, inScaleFactor, fmt::ptr(this));
  //检测出的结果进行绘制和存放到dsts中
  for (int i = 0; i < detectionMat.rows; i++)
  {
    //置值度获取
    float confidence = detectionMat.at<float>(i, 2);
    //如果大于阈值说明检测到人脸

    if (confidence > confidenceThreshold)
    {
      //计算矩形
      int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * tmpsrc.cols);
      int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * tmpsrc.rows);
      int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * tmpsrc.cols);
      int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * tmpsrc.rows);
      //生成矩形
      Rect rect((int)xLeftBottom, (int)yLeftBottom,
                (int)(xRightTop - xLeftBottom),
                (int)(yRightTop - yLeftBottom));

      //在原图上用红框画出矩形
      rectangle(*(request->getFrame()), rect, Scalar(0, 0, 255));
    }
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> runTime = end - start;
  VTF_LOGD("faceDected id{0} need {1}ms ", request->ID(), vtf::utils::TimeUtil::convertTime<std::chrono::milliseconds>(runTime).count());
  return true;
}

bool dnnfacedetect::detectFix(std::shared_ptr<FrameRequest> request)
{
  std::unique_lock<std::mutex> lk(m_mutex);

  auto frame = *(request->getFrame());
  //计算矩形
  int xLeftBottom = static_cast<int>(160);
  int yLeftBottom = static_cast<int>(120);
  int xRightTop = static_cast<int>(160 + 320);
  int yRightTop = static_cast<int>(120 + 240);
  //生成矩形
  Rect rect((int)xLeftBottom, (int)yLeftBottom,
            (int)(xRightTop - xLeftBottom),
            (int)(yRightTop - yLeftBottom));

  //在原图上用红框画出矩形
  rectangle(frame, rect, Scalar(0, 0, 255));
  std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
  return true;
}