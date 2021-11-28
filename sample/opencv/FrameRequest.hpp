/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-28 20:27:52
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 21:50:29
 */
#pragma once

#include <opencv2/opencv.hpp>
#include "../../src/core/pipeline/pipeRequest.hpp"


using namespace cv;
using namespace vtf::pipeline;


class FrameRequest : public PipeRequest {
public:
	FrameRequest(PipelineScenario scenario, Mat mat);

	~FrameRequest()
	{
		VTF_LOGD("frame request destory");
	}

	std::shared_ptr<Mat> getFrame() { return m_frame; }
	
private:
	std::shared_ptr<Mat> m_frame;
};

FrameRequest::FrameRequest(PipelineScenario scenario, Mat mat)
	:PipeRequest(scenario, true),
	 m_frame(std::make_shared<Mat>(mat))
{
    
}
