/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-03 21:22:29
 */
// #include "../../src/core/pipeline/PipeData.hpp"
#include "../../src/core/pipeline/pipeNodeDispatcher.hpp"
#include "../../src/core/pipeline/pipeline.hpp"
#include "../../src/core/notifier.hpp"
#include "../../src/core/utils.hpp"


#include "faceDected.hpp"
#include "FrameRequest.hpp"

#include <mutex>
#include<iostream>

#include <opencv2/opencv.hpp>
 
using namespace cv;
using namespace std;
using namespace vtf::pipeline;

enum CVTestScenario {
	PREVIEW,
	VIDEO,
};

std::mutex m_mutex;


// bool faceDected(std::shared_ptr<FrameRequest> request) 
// {

// 	auto start = std::chrono::system_clock::now();
// 	{
// 		std::unique_lock<std::mutex> lk(m_mutex);
// 		VTF_LOGD("faceDected process {0} start ", request->ID());
// 		VTF_LOGD("faceDected process {0} end ", request->ID());
// 	}
// 	auto end = std::chrono::system_clock::now();
// 	std::chrono::duration<double> runTime = end - start;
// 	VTF_LOGD("faceDected id{0} need {1}ms ", request->ID(), vtf::util::TimeUtil::convertTime<std::chrono::milliseconds>(runTime).count());
// 	return true;
// }

cv::VideoWriter w_cap;
double rate;

bool watermark(std::shared_ptr<FrameRequest> request)
{
	putText(*request->getFrame(), 
			"Hello, World!", 
			Point(200, 500),
			FONT_HERSHEY_COMPLEX, 1,
			Scalar(255, 255, 255),
			1, LINE_AA);
	
	VTF_LOGD("watermark finish {0}", request->ID());
	return true;
}

bool imageShowResultCallback(std::shared_ptr<FrameRequest> request)
{
	VTF_LOGD("result callback {0} ", request->ID());
	if (request->getNotifyStatus() == vtf::NotifyStatus::ERROR) {
		return true;
	}
	auto frame = request->getFrame();
	if (request->scenario() == CVTestScenario::VIDEO) {
		w_cap.write(*frame);
	} else if (CVTestScenario::PREVIEW) {
		imshow("window",*frame);  //在window窗口显示frame摄像头数据画面
	}
	return true;
}

std::shared_ptr<PipeLine<FrameRequest>> constructPipeline()
{
	std::shared_ptr<PipeLine<FrameRequest>> ppl = std::make_shared<PipeLine<FrameRequest>>();


	//add some node
	auto FDNode = ppl->addPipeNode(
		{
			.id = 1,
			.name = "FDNode",
			.pipelineScenarios = {CVTestScenario::PREVIEW},
			.processCallback = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
			.configProgress = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
		}
	);

	auto FDFixNode = ppl->addPipeNode(
		{
			.id = 2,
			.name = "FDNode",
			.pipelineScenarios = {CVTestScenario::VIDEO},
			.processCallback = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
			.configProgress = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
		}
	);

	auto WaterMarkNode = ppl->addPipeNode(
		{
			.id = 3,
			.name = "watermarkNode",
			.pipelineScenarios = {CVTestScenario::PREVIEW, CVTestScenario::VIDEO},
			.processCallback = watermark
		}
	);

	//connection
	FDNode->connect(WaterMarkNode);
	FDFixNode->connect(WaterMarkNode);

    ppl->addNotifier(
        {
            "pipeline_result_notifier",
            imageShowResultCallback,
            vtf::NotifierType::FINAL,
            8
        }
    );

	return ppl;
}
 
int main()
{
	auto ppl = constructPipeline();
	ppl->start();
	CVTestScenario curScenario = CVTestScenario::VIDEO;

	if (curScenario == CVTestScenario::PREVIEW) {
		VideoCapture capture(0);//读取视摄像头实时画面数据，0默认是笔记本的摄像头；如果是外接摄像头，这里改为1
		while (true)
		{
			Mat frame;
			capture >> frame;            //读取当前帧

			if(!frame.empty()){          //判断输入的视频帧是否为空的
				std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(curScenario, frame);
				ppl->submit(request);
			}
			if(waitKey(66) == 'q')   //延时33ms,获取用户是否按键的情况，如果按下q，会推出程序 
			break;
		}
		capture.release();     //释放摄像头资源
		destroyAllWindows();   //释放全部窗口
	} else if (curScenario == CVTestScenario::VIDEO) {
		Point pt1, pt2;
		Mat frame;
		cv::VideoCapture capture("/home/weiyanyu/learn/cpp/vtf/sample/bin/testVideo.mp4"); //关联读入视频文件
		if (!capture.isOpened())
		{
			std::cout << "fail to load video";
			return 1;
		}
		/*获取视频fps*/
		rate = capture.get(CAP_PROP_FPS);
		/*获取视频帧的尺寸*/
		int width = capture.get(CAP_PROP_FRAME_WIDTH);
		int height = capture.get(CAP_PROP_FRAME_HEIGHT);
		/*根据打开视频的参数初始化输出视频格式*/

		w_cap = cv::VideoWriter("re_video.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), rate, cv::Size(width, height));
		/*自定义输出视频的尺寸，需要将读取的视频帧尺寸进行变换，下文使用的resize函数完成*/
		//cv::VideoWriter w_cap("re_video.avi", CV_FOURCC('M', 'J', 'P', 'G'), rate, cv::Size(your.width, your.height));

		/*循环读取视频的帧*/
		while (capture.read(frame))
		{
			if (!frame.empty()) {
				std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(curScenario, frame);
				ppl->submit(request);
			}
			
			waitKey(int(1000 / rate));
		}
	}

	ppl->stop();
    return 0;
}