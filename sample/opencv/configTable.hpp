/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-12-05 19:18:44
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-10 22:52:45
 */

#include "../../src/core/pipeline/pipeline.hpp"
#include "../../src/core/notifier.hpp"
#include "FrameRequest.hpp"
#include "faceDected.hpp"

#include <opencv2/opencv.hpp>


enum CVTestScenario {
	PREVIEW = 100,
	VIDEO = 101,
};

cv::VideoWriter w_cap;

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

const static PipeLine<FrameRequest>::ConfigureTable configTable = 
{
    .pipeNodeCreateInfos = 
    {
        {
            .id = 1,
            .name = "FDNode",
            .pipelineScenarios = {CVTestScenario::PREVIEW},
            .processCallback = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
            .configProgress = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
        },
        {
            .id = 2,
            .name = "FDVideoNode",
            .pipelineScenarios = {CVTestScenario::VIDEO},
            .processCallback = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
            .configProgress = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
        },
        {
            .id = 3,
            .name = "watermarkNode",
            .pipelineScenarios = {CVTestScenario::PREVIEW, CVTestScenario::VIDEO},
            .processCallback = watermark
        }
    },
    .notifierCreateInfos = 
    {
        {
            1,
            "pipeline_result_notifier",
            imageShowResultCallback,
            vtf::NotifierType::FINAL,
            8
        }
    },
    .nodeConnections = 
    {
        {1, 3},
        {2, 3}
    }
};