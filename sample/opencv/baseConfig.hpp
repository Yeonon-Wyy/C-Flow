/*
 * @Author: Yeonon
 * @Date: 2022-09-03 21:01:42
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 21:15:54
 * @FilePath: /sample/opencv/baseConfig.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-09-03 21:01:42
 */
#pragma once

#include "../../src/core/type.hpp"

enum CVTestScenario
{
    PREVIEW = 100,
    VIDEO = 101,
};

enum CVNode
{
    FD_NODE,
    FDVIDEO_NODE,
    WATERMARK_NODE
};