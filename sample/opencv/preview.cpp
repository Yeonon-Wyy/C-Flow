/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 21:56:27
 */
// #include "../../src/core/pipeline/pipeRequest.hpp"
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

bool watermark(std::shared_ptr<FrameRequest> request)
{
	putText(*request->getFrame(), 
			"Hello, World!", 
			Point(20, 50),
			FONT_HERSHEY_COMPLEX, 1,
			Scalar(255, 255, 255),
			1, LINE_AA);
	return true;
}

bool resultCallback(std::shared_ptr<FrameRequest> request)
{
	// std::unique_lock<std::mutex> lk(mutex);
	VTF_LOGD("result callback");
	auto frame = request->getFrame();
	VTF_LOGD("show window frame {0}/{1}", frame->cols, frame->rows);
	imshow("window",*frame);  //在window窗口显示frame摄像头数据画面
	return true;
}

std::shared_ptr<PipeLine<FrameRequest>> constructPipeline(std::shared_ptr<dnnfacedetect> fd)
{
	std::shared_ptr<PipeLine<FrameRequest>> ppl = std::make_shared<PipeLine<FrameRequest>>();


	//add some node
	auto FDNode = ppl->addPipeNode(
		{
			.id = 1,
			.name = "FDNode",
			.pipelineScenarios = {CVTestScenario::PREVIEW},
			.processCallback = std::bind(&dnnfacedetect::detectFix, fd, std::placeholders::_1),
			.configProgress = std::bind(&dnnfacedetect::config, fd)
		}
	);

	auto WaterMarkNode = ppl->addPipeNode(
		{
			.id = 2,
			.name = "watermarkNode",
			.pipelineScenarios = {CVTestScenario::PREVIEW},
			.processCallback = watermark
		}
	);

	//connection
	FDNode->connect(WaterMarkNode);

    ppl->addNotifier(
        {
            "pipeline_result_notifier",
            resultCallback,
            vtf::NotifierType::FINAL,
            8
        }
    );

	return ppl;
}
 
int main()
{
    VideoCapture capture(0);//读取视摄像头实时画面数据，0默认是笔记本的摄像头；如果是外接摄像头，这里改为1
    std::shared_ptr<dnnfacedetect> fd = std::make_shared<dnnfacedetect>();
	auto ppl = constructPipeline(fd);
	ppl->start();
    while (true)
    {
    	Mat frame;
        capture >> frame;            //读取当前帧

        if(!frame.empty()){          //判断输入的视频帧是否为空的
			std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(CVTestScenario::PREVIEW, frame);
			ppl->submit(request);
        }
        if(waitKey(33) == 'q')   //延时33ms,获取用户是否按键的情况，如果按下q，会推出程序 
		break;
    }
	ppl->stop();
    capture.release();     //释放摄像头资源
    destroyAllWindows();   //释放全部窗口
    return 0;
}