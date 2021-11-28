/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-27 22:24:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 19:27:41
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>

using namespace std;
using namespace cv;

class dnnfacedetect
{
private:
  string _modelbinary, _modeldesc;
  dnn::Net _net;

public:
  //构造函数 传入模型文件
  dnnfacedetect();
  dnnfacedetect(string modelBinary, string modelDesc);

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
  void detect(Mat frame);
};

dnnfacedetect::dnnfacedetect()
{
  dnnfacedetect("", "");
}

//构造函数
dnnfacedetect::dnnfacedetect(string modelBinary, string modelDesc)
{
  _modelbinary = modelBinary;
  _modeldesc = modelDesc;

  //初始化置信阈值
  confidenceThreshold = 0.6;
  inScaleFactor = 0.5;
  inWidth = 300;
  inHeight = 300;
  meanVal = Scalar(104.0, 177.0, 123.0);
}

dnnfacedetect::~dnnfacedetect()
{
  _net.~Net();
}

//初始化dnnnet
bool dnnfacedetect::initdnnNet()
{
  _net = dnn::readNetFromTensorflow(_modelbinary, _modeldesc);
  _net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
  _net.setPreferableTarget(dnn::DNN_TARGET_CPU);

  return !_net.empty();
}

//人脸检测
void dnnfacedetect::detect(Mat frame)
{
  Mat tmpsrc = frame;
  // 修改通道数
  if (tmpsrc.channels() == 4)
    cvtColor(tmpsrc, tmpsrc, COLOR_BGRA2BGR);
  // 输入数据调整
  Mat inputBlob = dnn::blobFromImage(tmpsrc, inScaleFactor,
                                     Size(inWidth, inHeight), meanVal, false, false);
  _net.setInput(inputBlob, "data");

  //人脸检测
  Mat detection = _net.forward("detection_out");

  Mat detectionMat(detection.size[2], detection.size[3],
                   CV_32F, detection.ptr<float>());

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
      rectangle(frame, rect, Scalar(0, 0, 255));
    }
  }
}