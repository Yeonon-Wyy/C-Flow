/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 19:55:33
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipeNodeDispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"
#include "../core/notifier.hpp"
#include "../core/utils.hpp"
#include "faceDected.hpp"

#include <mutex>
#include<iostream>

#include <opencv2/opencv.hpp>
 
using namespace cv;
using namespace std;
using namespace vtf::pipeline;

enum CVTestScenario {
	PREVIEW,
};

class FrameRequest : public PipeRequest {
public:
	FrameRequest(PipelineScenario scenario, Mat mat, std::shared_ptr<dnnfacedetect> fd)
		:vtf::pipeline::PipeRequest(scenario, true),
		 m_frame(std::make_shared<Mat>(mat)),
		 m_fd(fd)
	{}

	~FrameRequest()
	{
		VTF_LOGD("frame request destory");
	}

	std::shared_ptr<Mat> getFrame() { return m_frame; }
	std::shared_ptr<dnnfacedetect> getFD() { return m_fd; }
	
private:
	std::shared_ptr<Mat> m_frame;
	std::shared_ptr<dnnfacedetect> m_fd;
};

std::mutex m_mutex;


bool faceDected(std::shared_ptr<FrameRequest> request) 
{
	std::shared_ptr<Mat> frame = request->getFrame();

	auto start = std::chrono::system_clock::now();
	{
		std::unique_lock<std::mutex> lk(m_mutex);
		VTF_LOGD("faceDected process {0} start ", request->ID());
		request->getFD()->detect(*frame);
		VTF_LOGD("faceDected process {0} end ", request->ID());
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> runTime = end - start;
	VTF_LOGD("faceDected id{0} need {1}ms ", request->ID(), vtf::util::TimeUtil::convertTime<std::chrono::milliseconds>(runTime).count());
	return true;
}

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

std::shared_ptr<PipeLine<FrameRequest>> constructPipeline()
{
	std::shared_ptr<PipeLine<FrameRequest>> ppl = std::make_shared<PipeLine<FrameRequest>>();


	//add some node
	auto FDNode = ppl->addPipeNode(
		{
			.id = 1,
			.name = "FDNode",
			.pipelineScenarios = {CVTestScenario::PREVIEW},
			.processCallback = faceDected
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
    
	char filepath[256];
	getcwd(filepath, sizeof(filepath));
	VTF_LOGD("filePath: {0}", filepath);
	//定义模型文件
	string ModelBinary = (string)filepath + "/opencv_face_detector_uint8.pb";
	string ModelDesc = (string)filepath + "/opencv_face_detector.pbtxt";

	auto fdetect = std::make_shared<dnnfacedetect>(ModelBinary, ModelDesc);


    if (!fdetect->initdnnNet())
    {
      	return -1;
    }

	auto ppl = constructPipeline();
	ppl->start();
    while (true)
    {
    	Mat frame;
        capture >> frame;            //读取当前帧

        if(!frame.empty()){          //判断输入的视频帧是否为空的
			std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(CVTestScenario::PREVIEW, frame, fdetect);
			ppl->submit(request);
        }
        if(waitKey(33) == 'q')   //延时20ms,获取用户是否按键的情况，如果按下q，会推出程序 
		break;
    }
	ppl->stop();
    capture.release();     //释放摄像头资源
    destroyAllWindows();   //释放全部窗口
    return 0;
}