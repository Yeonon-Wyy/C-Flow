/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-28 20:27:52
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 19:33:55
 */
#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv4/opencv2/opencv.hpp>
#include <vector>

#include "../../src/core/pipeline/pipedata.hpp"

using namespace cv;
using namespace cflow::pipeline;

class FrameRequest : public PipeData
{
public:
    FrameRequest(PipelineScenario scenario, Mat mat);

    ~FrameRequest() { CFLOW_LOGD("frame request destory"); }

    std::shared_ptr<Mat> getFrame() { return m_frame; }

    void addFaceRect(Rect& FaceRect) { m_faces.push_back(FaceRect); }
    void addEyeRect(Rect& eyeRect) { m_eyes.push_back(eyeRect); }

    std::vector<Rect> getFaceRects() const { return m_faces; }
    std::vector<Rect> getEyeRects() const { return m_eyes; }

private:
    std::shared_ptr<Mat> m_frame;
    std::vector<Rect> m_faces;
    std::vector<Rect> m_eyes;
};

FrameRequest::FrameRequest(PipelineScenario scenario, Mat mat) : PipeData(scenario, true), m_frame(std::make_shared<Mat>(mat)) {}
