/*
 * @Author: Yeonon
 * @Date: 2022-09-03 19:06:49
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 21:03:48
 * @FilePath: /sample/opencv/watermark.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-09-03 19:06:49
 */
#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

#include "FrameRequest.hpp"

using namespace cv;

bool watermark(std::shared_ptr<FrameRequest> request)
{
    CFLOW_LOGD("watermark start {0}", request->ID());
    TRACE_FUNC_ID_START(__FUNCTION__, request->ID());
    auto frame = *request->getFrame();
    cv::Point p = cv::Point(0, 20);
    //加上字符的起始点
    cv::putText(frame, "C-Flow watermark copyrigiht", p,
                cv::FONT_HERSHEY_TRIPLEX, 0.8, cv::Scalar(255, 200, 200), 2);

    return true;
}