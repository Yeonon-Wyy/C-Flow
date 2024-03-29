## CFlow

### 1. 概述

"C-Flow"是是源代码集合，使用Header-only的方式组织代码结构，同时他也可以是一个框架/引擎/三方库，无所谓如何定义他是什么，能帮助我们提升效率即可。

"C-Flow"的目标是：在用户需要将大任务拆分成小任务时，可高效批量处理这些小任务。如果这些小任务之间存在依赖关系，那么也没问题，"C-Flow"支持用户设定任务之间的依赖关系，按照用户想要的顺序去执行小任务。具备这种特征的常见的业务有：图像处理（例如Camera，游戏引擎），网络服务，大数据批量处理等等。

目前支持以下两种任务流模型：

1. taskflow，即以小任务为基本单位构建的一个大任务，小任务之间可能存在依赖关系，存在前置依赖的任务需要等到前置任务完成之后才能执行，而不存在依赖关系的任务则可并行执行。使用CFlow，用户仅需要定义任务处理逻辑以及任务之间的一个依赖关系，CFlow即可帮助用户完成上述流程。
2. pipeline，即“流水线模型”（在图形学领域也被叫做“管线”）。该模型主要用于处理流式数据，输入源像水流一样源源不断。通过将数据处理逻辑拆分为多个不同的子逻辑，数据流过某一个子逻辑不会阻塞其他子逻辑。从而使得无论有多少个子流程，数据都能以与输入相近的速率进行输出，提升整体效率。在CFlow中，用户也可以通过定义子逻辑以及子逻辑之间的依赖关系（即子流程的处理顺序）来实现流水线模型。

### 2. 整体架构设计

> 项目还处于 需求->开发（写BUG）->Debug->需求->开发（写BUG）->Debug.....这样的一个循环中.....，以下架构图和流程图具有一定的时效性。

#### 2.1 架构
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/cflow.png)
#### 2.2 DAG
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/DAG.jpg)
#### 2.3 pipeline flow
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/piepline_flow.jpg)

### 3. 简单使用

#### 3.1 taskflow

```c++
//声明定义一个TaskFlowCtl对象
cflow::task::TaskFlowCtl flowCtl(true);
using BufferQ = cflow::BlockingQueue<Buffer>;

//通过addTaskWithTaskInfo接口，添加一个task
auto task1 = flowCtl.addTaskWithTaskInfo(
    {cflow::task::TaskPriority::NORMAL,"task1"}, 
    [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
		//logic
    },
    inputBufferQ,
    outputBufferQ
);

//通过addTaskWithTaskInfo接口，添加一个task
auto task2 = flowCtl.addTaskWithTaskInfo(
    {cflow::task::TaskPriority::NORMAL,"task1"}, 
    [](std::shared_ptr<BufferQ> inbufferQ) {
		//logic
    },
    inputBufferQ
);

auto task3 = flowCtl.addTaskWithTaskInfo(
    {cflow::task::TaskPriority::NORMAL,"task1"}, 
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

#### 3.2 pipeline

```c++
//用户自定义类型，需要继承框架的PipeTask类
class FrameRequest : public PipeTask {
public:
	FrameRequest(PipelineScenario scenario, Mat mat);

	~FrameRequest()
	{
		CFLOW_LOGD("frame request destory");
	}

	std::shared_ptr<Mat> getFrame() { return m_frame; }
	
private:
	std::shared_ptr<Mat> m_frame;
};
//填写配置表
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
            .pipelineScenarios = {},
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
                .name = "pipeline_node_done_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) {
                        if (request->getNotifyStatus() == cflow::NotifyStatus::ERROR) {
                            CFLOW_LOGE("node done {0} notify ERROR", request->ID());
                        } else {
                            CFLOW_LOGE("node done {0} notify OK", request->ID());
                        }
                        return true;
                },
                .configProgress = []() {
                    CFLOW_LOGD("pipeline_node_done_notifier - user define config");  
                },
                .stopProgress = []() {
                    CFLOW_LOGD("pipeline_node_done_notifier - user define stop");
                },
                .type = cflow::NotifierType::TASK_LISTEN,
            },
            {
                .id = 2,
                .name = "pipeline_result_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) {
                        if (request->getNotifyStatus() == cflow::NotifyStatus::ERROR) {
                            CFLOW_LOGE("result {0} notify ERROR", request->ID());
                        } else {
                            CFLOW_LOGE("result {0} notify OK", request->ID());
                        }
                        return true;
                },
                .type = cflow::NotifierType::FINAL,
            }
        },
    .nodeConnections = 
    {
        {1, 3},
        {2, 3}
    }
};

auto ppl = PipeLine<PipelineRequest>::generatePipeLineByConfigureTable(table);
//启动pipeline
ppl->start();

//构造一个request，这里FrameRequest也是用户自定义的，但必须继承cflow::pipeline::Task
//VYF提供了一个cflow::pipeline::Task的默认实现cflow::pipeline::PipeTask，实现了构建依赖等必须的接口，用户可选择继承cflow::pipeline::Task之后加上自己的逻辑即可，也可以直接继承cflow::pipeline::Task，但必须实现cflow::pipeline::Task的接口
std::shared_ptr<FrameRequest> request = std::make_shared<FrameRequest>(curScenario, frame);

//PipeTask还提供了addNotifierForNode接口可为当前task path中的某个node指定notifier
//参数为nodeId和notifierID
req->addNotifierForNode(1, 1);

//将数据submit到pipiline即可
ppl->submit(request);

//停止pipeline，调用之后会停止接受任何数据，并等待所有已submit的数据执行完当前的node，如果整个流程没有完成，则会notifi error，如果某个node block，整个程序将也会被block
ppl->stop();
```

其中,配置表的详细信息如下：

1. maxProcessingSize代表每一个node的最大可处理数量，即node里的队列的大小

2. threadPoolSize代表整个pipeline中线程池的大小

3. pipeNodeCreateInfos包含各个node的详细信息
   
   - id表示node id，id不可重复，重复则会被覆盖
   
   - name表示node name，可根据业务自定义，方便日志中展示
   
   - pipelineScenarios，表示该node在哪些scenarios中使用，其中scenarios也是用户自定义的，对于框架来说，他只是一个类似id的东西用于表示某一具体的场景。这个参数非常重要，框架会根据scenarios来串联node得到一条pipeline。
   
   - processCallback，即node的处理函数，函数的具体实现由用户提供。用户可实现复杂的逻辑，最后在这个函数里调用，随心所欲吧。
   
   - configProgress，提供一个node初始化的回调函数，用户可以在这个函数里初始化一些参数之类的。当然如果不需要的话，不提供也行，一切看用户的想法。

4. notifierCreatInfo，后处理函数，例如人脸检测完毕后统计一些信息等后处理操作。支持两种类似，一种是node后处理（TASK_LISTEN），一种是task执行完所有node之后的后处理（FINAL）。TASK_LISTEN是在单个node处理完毕后执行一次，FINAL则是在task执行完所有node之后执行一次。

5. nodeConnections，表示node的连接关系。框架会通过连接关系，执行拓扑排序结合node scenarios信息得到若干个pipeline，在Pipeline::submit中根据用户指定的scenarios选择对应的pipeline来执行。nodeConnecions的连接信息得到的图必须是有向无环图，否则框架将会终止，这是框架为数不多的强制性限制之一。

> 更详细的例子请查看/sample/opencv/，这个demo实现了读取视频流数据，对每一帧做人脸检测，水印等处理，最终合成新的视频。

### 4. 如何编译

#### 4.1 下载源码后直接include头文件
C-Flow使用Header-only风格进行编写，只需要获取/src目录下的源码，并在项目中include需要的头文件即可。当前不支持编译成独立的静态库或者共享库。

#### 4.2 通过cmake
另一种方式则是将cflow的头文件install到本地，在项目根目录下执行：
```shell
mkdir build & cd build
cmake ..
cmake --build . & make install
```
默认会把项目文件放入/usr/local/include目录下，后续直接使用即可。

下面是一个以cmake的方式使用的例子，以供参考：
```shell
cmake_minimum_required(VERSION 2.6)
project(cflow_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER             "/usr/bin/g++")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DCFlow_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

# user-define log file path
add_definitions( -DLOGFILENAME="/tmp/cflow_log/cflow_log.txt")

# include cflow
find_package(cflow CONFIG REQUIRED)
include_directories(${cflow_INCLUDE_DIR})

# include /usr/include
include_directories("/usr/include/")

# include thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

SET(EXECUTABLE_OUTPUT_PATH "../bin/")


# if need tracy profiler
if(DEFINED ENV{NEED_TRACY_PROFILER})
    # include tracy
    add_subdirectory(${cflow_INCLUDE_DIR}/cflow/3rdparty/tracy .)

    # set dependency
    
    set(TARGET_DEPENDCY 
        PUBLIC Threads::Threads
        cflow::cflow
        Tracy::TracyClient
    )
else()
    # set dependency
    set(TARGET_DEPENDCY 
        PUBLIC Threads::Threads
        cflow::cflow
    )
endif()


# add executable
add_executable(testPipeline "../testPipeline.cpp")
target_link_libraries(testPipeline ${TARGET_DEPENDCY})
```

### 5. dump graph with graphviz
#### 5.1 Dump whole graph
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/all_pipeline.png)

#### 5.2 Dump Scenario 100
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/pipeline_100.png)

#### 5.3 Dump Scenario 101
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/pipeline_101.png)

#### 5.4 Dump Scenario 102
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/pipeline_102.png)

#### 5.5 Dump Scenario 103
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/pipeline_103.png)

#### 5.6 dump task flow
![image](https://github.com/Weiyanyu/C-Flow/blob/main/doc/Task_flow_control.png)

### 6. 依赖
以下依赖的三方库由于构建方式不同，所以cflow的集成方式也有些不同，请注意。

1. spdlog，强大的日志系统，按照spdlog推荐的方式将spdlog install到usr/local/include下
2. tracy，强大的profiler，其提供的frameMark功能适合cflow的数据流模型，按照tracy推荐的方式直接集成到项目的code base中。并放入了/src/3rdparty目录下，但git ignore忽略了它，如果需要构建cflow，则需要下载tracy源码并放入/src/3rdparty中


### 6. 支持的功能
> 待补充


### 7. 后续计划

//正在思考中
