/*
 * @Author: Yeonon
 * @Date: 2022-09-03 20:59:39
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-12 17:47:47
 * @FilePath: /sample/opencv/resultProcess.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-09-03 20:59:39
 */

#pragma once

#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>

#include "FrameRequest.hpp"
#include "baseConfig.hpp"

cv::VideoWriter w_cap;
std::mutex m_mutex;

using namespace cv;

bool imageShowResultCallback(std::shared_ptr<FrameRequest> request)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    TRACE_FUNC_ID_START(__FUNCTION__, request->ID());
    if (request->getNotifyStatus() == cflow::NotifyStatus::ERROR)
    {
        return true;
    }
    auto frame = request->getFrame();
    if (request->scenario() == CVTestScenario::VIDEO)
    {
        // write face rect
        for (auto&& face : request->getFaceRects())
        {
            CFLOW_LOGD("result face roi [{0},{1},{2},{3}]", face.x, face.y,
                       face.width, face.height);

            rectangle(*frame, Point(face.x, face.y),
                      Point(face.x + face.width, face.y + face.height),
                      Scalar(0, 0, 255), 1, 8);
        }
    }
    else if (request->scenario() == CVTestScenario::PREVIEW)
    {
        imshow("window", *frame); //在window窗口显示frame摄像头数据画面
    }
    {
        w_cap.write(*frame);
    }
    return true;
}