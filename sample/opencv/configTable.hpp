/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-12-05 19:18:44
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-11 16:08:47
 */

#include <mutex>
#include <opencv2/opencv.h>

#include "../../src/core/notifier.h"
#include "../../src/core/pipeline/pipeline.h"
#include "../../src/core/utils/log/trace_log.h"

#include "watermark.h"
#include "FrameRequest.h"
#include "faceDected.h"
#include "resultProcess.h"
#include "baseConfig.h"

using namespace cv;

// clang-format off
const static PipeLine<FrameRequest>::ConfigureTable configTable = 
{
    .maxProcessingSize   = 40,
    .pipeNodeCreateInfos = 
    {
        {   .id                = FD_NODE,
            .name              = "FDNode",
            .pipelineScenarios = {CVTestScenario::PREVIEW},
            .processCallback   = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
            .configProgress    = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
        },
        {   .id                = FDVIDEO_NODE,
            .name              = "FDVideoNode",
            .pipelineScenarios = {CVTestScenario::VIDEO},
            .processCallback   = std::bind(&dnnfacedetect::detect, dnnfacedetect::getInstance(), std::placeholders::_1),
            .configProgress    = std::bind(&dnnfacedetect::config, dnnfacedetect::getInstance())
        },
        {   .id                = WATERMARK_NODE, 
            .name              = "watermarkNode", 
            .pipelineScenarios = {CVTestScenario::PREVIEW, CVTestScenario::VIDEO}, 
            .processCallback   = watermark
        }
    },
    .notifierCreateInfos = 
    {
    {
        .id              = 1,
        .name            = "pipeline_result_notifier",
        .processCallback = imageShowResultCallback,
        .type            = cflow::NotifierType::FINAL,
    }
    },
    .nodeConnections     = 
    {
        {FD_NODE, WATERMARK_NODE}, 
        {FDVIDEO_NODE, WATERMARK_NODE}
    }
};
// clang-format on