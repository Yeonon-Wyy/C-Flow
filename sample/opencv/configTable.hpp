/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-12-05 19:18:44
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-06 20:27:22
 */

#include "../../src/core/pipeline/pipeline.hpp"
#include "../../src/core/notifier.hpp"
#include "FrameRequest.hpp"
#include "faceDected.hpp"
#include <mutex>

#include <opencv2/opencv.hpp>


using namespace cv;

enum CVTestScenario {
	PREVIEW = 100,
	VIDEO = 101,
};

cv::VideoWriter w_cap;
std::mutex m_mutex;

Mat logo = imread("/home/yeonon/learn/cpp/vtf/sample/bin/logo.png");

bool watermark(std::shared_ptr<FrameRequest> request)
{

    auto frame = *request->getFrame();

    Rect roi(frame.cols*0.7, frame.rows*0.7, frame.cols/4, frame.rows/4);
    Mat frame_roi = frame(roi);

    resize(logo, logo, Size(frame.cols/4, frame.rows/4));
    addWeighted(frame_roi,0, logo,1,1, frame_roi);
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

	} else if (request->scenario() == CVTestScenario::PREVIEW) {
		imshow("window",*frame);  //在window窗口显示frame摄像头数据画面
	}
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        w_cap.write(*frame);
    }
	return true;
}

const static PipeLine<FrameRequest>::ConfigureTable configTable = 
{
    .maxProcessingSize = 8,
    .threadPoolSize = 50,
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
            .id = 1,
            .name = "pipeline_result_notifier",
            .processCallback = imageShowResultCallback,
            .type = vtf::NotifierType::FINAL,
        }
    },
    .nodeConnections = 
    {
        {1, 3},
        {2, 3}
    }
};