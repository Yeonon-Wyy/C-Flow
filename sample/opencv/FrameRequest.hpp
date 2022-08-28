/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-28 20:27:52
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-27 19:41:14
 */
#pragma once

#include "../../src/core/pipeline/pipedata.hpp"
#include <opencv4/opencv2/opencv.hpp>

using namespace cv;
using namespace vtf::pipeline;

class FrameRequest : public PipeData {
public:
  FrameRequest(PipelineScenario scenario, Mat mat);

  ~FrameRequest() { VTF_LOGD("frame request destory"); }

  std::shared_ptr<Mat> getFrame() { return m_frame; }

private:
  std::shared_ptr<Mat> m_frame;
};

FrameRequest::FrameRequest(PipelineScenario scenario, Mat mat)
    : PipeData(scenario, true), m_frame(std::make_shared<Mat>(mat)) {}
