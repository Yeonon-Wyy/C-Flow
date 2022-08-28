/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-27 20:25:47
 */
#include "configTable.hpp"
#include <opencv2/opencv.hpp>

#include <mutex>

using namespace cv;
using namespace std;
using namespace vtf::pipeline;
double rate;

int main() {

  auto ppl =
      PipeLine<FrameRequest>::generatePipeLineByConfigureTable(configTable);
  ppl->start();
  CVTestScenario curScenario = CVTestScenario::VIDEO;

  if (curScenario == CVTestScenario::PREVIEW) {
    auto outFile = "/home/weiyanyu/learn/cpp/vtf/sample/bin/test.avi";
    VideoCapture capture(
        0); //读取视摄像头实时画面数据，0默认是笔记本的摄像头；如果是外接摄像头，这里改为1

    int w = static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT));
    w_cap = cv::VideoWriter("test.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'),
                            1000 / 66, cv::Size(w, h));
    while (true) {
      Mat frame;
      capture >> frame; //读取当前帧

      if (!frame.empty()) { //判断输入的视频帧是否为空的
        std::shared_ptr<FrameRequest> request =
            std::make_shared<FrameRequest>(curScenario, frame);
        ppl->submit(request);
      }
      if (waitKey(66) ==
          'q') //延时33ms,获取用户是否按键的情况，如果按下q，会推出程序
        break;
    }
    capture.release(); //释放摄像头资源
    w_cap.release();
    destroyAllWindows(); //释放全部窗口
  } else if (curScenario == CVTestScenario::VIDEO) {
    Point pt1, pt2;
    Mat frame;
    cv::VideoCapture capture("/home/yeonon/learn/cpp/vtf/sample/bin/"
                             "testVideo.mp4"); //关联读入视频文件
    if (!capture.isOpened()) {
      std::cout << "fail to load video";
      return 1;
    }
    /*获取视频fps*/
    rate = capture.get(CAP_PROP_FPS);
    VTF_LOGD("video rate {0}", rate);
    /*获取视频帧的尺寸*/
    int width = capture.get(CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CAP_PROP_FRAME_HEIGHT);
    /*根据打开视频的参数初始化输出视频格式*/

    w_cap =
        cv::VideoWriter("re_video.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'),
                        rate, cv::Size(width, height));
    /*自定义输出视频的尺寸，需要将读取的视频帧尺寸进行变换，下文使用的resize函数完成*/
    // cv::VideoWriter w_cap("re_video.avi", CV_FOURCC('M', 'J', 'P', 'G'),
    // rate, cv::Size(your.width, your.height));

    /*循环读取视频的帧*/
    while (capture.read(frame)) {
      if (!frame.empty()) {
        std::shared_ptr<FrameRequest> request =
            std::make_shared<FrameRequest>(curScenario, frame);
        ppl->submit(request);
      }

      waitKey(int(1000 / rate));
    }
    w_cap.release();
  }

  ppl->stop();
  return 0;
}