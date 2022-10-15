/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-15 20:02:20
 */
#include <mutex>
#include <opencv2/opencv.hpp>
#include <pthread.h>

#include "configTable.hpp"
#include "../../src/core/utils/thread/thread_utils.hpp"

using namespace cv;
using namespace std;
using namespace cflow::pipeline;

void startPipeline()
{
    auto ppl =
        PipeLine<FrameRequest>::generatePipeLineByConfigureTable(configTable);
    ppl->start();
    CVTestScenario curScenario = CVTestScenario::VIDEO;

    if (curScenario == CVTestScenario::PREVIEW)
    {
        VideoCapture capture(
            0); //读取视摄像头实时画面数据，0默认是笔记本的摄像头；如果是外接摄像头，这里改为1

        int w = static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH));
        int h = static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT));
        w_cap =
            cv::VideoWriter("test.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'),
                            1000 / 66, cv::Size(w, h));
        while (true)
        {
            Mat frame;
            capture >> frame; //读取当前帧

            if (!frame.empty())
            { //判断输入的视频帧是否为空的
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
    }
    else if (curScenario == CVTestScenario::VIDEO)
    {
        Point pt1, pt2;
        Mat frame;
        std::string inputFilename =
            "/home/yeonon/learn/cpp/cflow/sample/bin/testVideo.mp4";
        std::string outputFilename = "re_video.avi";
        cv::VideoCapture capture(inputFilename); //关联读入视频文件
        if (!capture.isOpened())
        {
            CFLOW_LOGE("can't open video file {0}", inputFilename);
            return;
        }
        /*获取视频fps*/
        double rate = capture.get(CAP_PROP_FPS);
        CFLOW_LOGD("video rate {0}", rate);
        /*获取视频帧的尺寸*/
        int width = capture.get(CAP_PROP_FRAME_WIDTH);
        int height = capture.get(CAP_PROP_FRAME_HEIGHT);
        /*根据打开视频的参数初始化输出视频格式*/

        w_cap = cv::VideoWriter(outputFilename,
                                VideoWriter::fourcc('M', 'J', 'P', 'G'), rate,
                                cv::Size(width, height));
        /*自定义输出视频的尺寸，需要将读取的视频帧尺寸进行变换，下文使用的resize函数完成*/

        /*循环读取视频的帧*/
        while (capture.read(frame))
        {
            if (!frame.empty())
            {
                std::shared_ptr<FrameRequest> request =
                    std::make_shared<FrameRequest>(curScenario, frame);
                if (request->ID() % 2 == 0)
                {
                    request->skipPipeNode(FDVIDEO_NODE);
                }
                ppl->submit(request);
            }

            waitKey(int(1000 / rate));
        }
        w_cap.release();
    }

    ppl->stop();
}

int main()
{
    std::thread t1(startPipeline);
    cflow::utils::thread::setScheduling(t1, SCHED_FIFO, 0);
    t1.join();
}
