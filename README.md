## VTF

### 1. 概述

"VTF"是"very tiny flow"的首字母缩写，这是一个致力于为任何符合“任务流”模型的业务提供高效，方便的接口的一个“引擎“。

目前支持以下两种任务流模型：

1. taskflow，即以小任务为基本单位构建的一个大任务，小任务之间可能存在依赖关系，存在前置依赖的任务需要等到前置任务完成之后才能执行，而不存在依赖关系的任务则可并行执行。使用VTF，用户仅需要定义任务处理逻辑以及任务之间的一个依赖关系，VTF即可帮助用户完成上述流程。
2. pipeline，即“流水线模型”（在图形学领域也被叫做“管线”）。该模型主要用于处理流式数据，输入源像水流一样源源不断。通过将数据处理逻辑拆分为多个不同的子逻辑，数据流过某一个子逻辑不会block其他子逻辑。从而使得无论有多少个子流程，数据都能以与输入相近的速率进行输出，提升整体效率。在VTF中，用户也可以通过定义子逻辑以及子逻辑之间的依赖关系（即子流程的处理顺序）来实现流水线模型。

### 2. 简单使用

#### 2.1 taskflow

```c++
//声明定义一个TaskFlowCtl对象
vtf::task::TaskFlowCtl flowCtl(true);
using BufferQ = vtf::BlockingQueue<Buffer>;

//通过addTaskWithTaskInfo接口，添加一个task
auto task1 = flowCtl.addTaskWithTaskInfo(
    {vtf::task::TaskPriority::NORMAL,"task1"}, 
    [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
		//logic
    },
    inputBufferQ,
    outputBufferQ
);

//通过addTaskWithTaskInfo接口，添加一个task
auto task2 = flowCtl.addTaskWithTaskInfo(
    {vtf::task::TaskPriority::NORMAL,"task1"}, 
    [](std::shared_ptr<BufferQ> inbufferQ) {
		//logic
    },
    inputBufferQ
);

auto task3 = flowCtl.addTaskWithTaskInfo(
    {vtf::task::TaskPriority::NORMAL,"task1"}, 
    [](std::shared_ptr<BufferQ> inbufferQ) {
		//logic
    },
    inputBufferQ
);

//task2依赖task1
//task3依赖task1
//但task2和task3并无依赖关系，故task2和task3将会并发执行
task1->connect(task2);
task1->connect(task3);

//将会block直到所有任务完成
flowCtl.start();
```

#### 2.2 pipeline

```c++
//用户自定义类型，需要继承框架的PipeData类
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
//填写配置表
const static PipeLine<FrameRequest>::ConfigureTable configTable = 
{
    .queueSize = 8,
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
            1,
            "pipeline_result_notifier",
            imageShowResultCallback,
            vtf::NotifierType::FINAL,
            8
        }
    },
    .nodeConnections = 
    {
        //src->dst
        {1, 3},
        {2, 3}
    }
};

auto ppl = PipeLine<PipelineRequest>::generatePipeLineByConfigureTable(table);
//启动pipeline
ppl->start();

//构造一个request，这里FrameRequest也是用户自定义的，但必须继承vtf::pipeline::Data
//VYF提供了一个vtf::pipeline::Data的默认实现vtf::pipeline::PipeData，实现了构建依赖等必须的接口，用户可选择继承vtf::pipeline::Data之后加上自己的逻辑即可，也可以直接继承vtf::pipeline::Data，但必须实现vtf::pipeline::Data的接口
std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(curScenario, frame);

//PipeData还提供了addNotifierForNode接口可为当前data path中的某个node指定notifier
//参数为nodeId和notifierID
req->addNotifierForNode(1, 1);

//将数据submit到pipiline即可
ppl->submit(request);

//停止pipeline，调用之后会停止接受任何数据，并等待所有已submit的数据执行完当前的node，如果整个流程没有完成，则会notifi error，如果某个node block，整个程序将也会被block
ppl->stop();
```

### 3. 整体架构设计

//正在撰写中

### 4. 下一个版本计划

//正在思考中