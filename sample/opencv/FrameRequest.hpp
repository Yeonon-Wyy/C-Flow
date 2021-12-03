/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-28 20:27:52
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-03 21:16:52
 */
#pragma once

#include <opencv2/opencv.hpp>
#include "../../src/core/pipeline/pipeData.hpp"


using namespace cv;
using namespace vtf::pipeline;


class FrameRequest : public PipeData {
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
	:PipeData(scenario, true),
	 m_frame(std::make_shared<Mat>(mat))
{
    
}
