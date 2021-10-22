/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-22 23:42:06
 */
#include "../core/taskflowctl.hpp"
#include "../core/utils.hpp"
#include <chrono>
#include "../core/blocking_queue.hpp"



using Buffer = std::vector<std::vector<int>>;
using BufferQ = vtf::BlockingQueue<Buffer>;

int main()
{
    vtf::TaskFlowCtl flowCtl(true);

    std::shared_ptr<BufferQ> p1MasterbufferQ = std::make_shared<BufferQ>(8);
    std::shared_ptr<BufferQ> p1SlavebufferQ = std::make_shared<BufferQ>(8);
    std::shared_ptr<BufferQ> P2MasterbufferQ = std::make_shared<BufferQ>(8);
    std::shared_ptr<BufferQ> P2SlavebufferQ = std::make_shared<BufferQ>(8);
    std::shared_ptr<BufferQ> mdp1bufferQ = std::make_shared<BufferQ>(8);

    std::shared_ptr<BufferQ> resbufferQ = std::make_shared<BufferQ>(8);



    auto P1NodeMaster = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P1NodeMaster"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->pop();
            
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
        },
        p1MasterbufferQ,
        P2MasterbufferQ
    );

    auto P1NodeSlave = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P1NodeSlave"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->pop();
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
        },
        p1SlavebufferQ,
        P2SlavebufferQ
    );

    auto P2SMasterNode = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P2SMasterNode"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->pop();
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
        },
        P2MasterbufferQ,
        mdp1bufferQ
    );

    auto P2SSlaveNode = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P2SSlaveNode"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->pop();
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
        },
        P2SlavebufferQ,
        mdp1bufferQ
    );

    auto mdpNode1 = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"mdpNode1"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            while (!inbufferQ->isEmpty()) {
                auto buffer = inbufferQ->pop();
                for (int col = 0; col <4; col++) {
                    buffer[0][col] *= 2;
                }
                outBufferQ->push(buffer);
                std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));

            }
        },
        mdp1bufferQ,
        resbufferQ
    );



    P1NodeMaster->precede(P2SMasterNode);
    P1NodeSlave->precede(P2SSlaveNode);
    P2SMasterNode->precede(mdpNode1);
    P2SSlaveNode->precede(mdpNode1);

    // auto start = std::chrono::system_clock::now();
    // flowCtl.start();
    // auto end = std::chrono::system_clock::now();
    // VTF_LOGI << "all task need: " << vtf::util::TimeUtil::convertTime<std::chrono::milliseconds>(end - start).count() << "ms" ;

    // VTF_LOGI << "out buffer content: ";

    std::thread dataSourceThread(
        [&flowCtl](std::shared_ptr<BufferQ> masterBufferQ, std::shared_ptr<BufferQ> slaveBufferQ, std::shared_ptr<BufferQ> resbufferQ){
            for (int i = 0; i < 10; i++) {
                Buffer masertBuffer;
                masertBuffer.push_back({1,1,1,1});
                masertBuffer.push_back({1,1,1,1});
                masertBuffer.push_back({1,1,1,1});
                masertBuffer.push_back({1,1,1,1});


                Buffer slaveBuffer;
                slaveBuffer.push_back({1,1,1,1});
                slaveBuffer.push_back({1,1,1,1});
                slaveBuffer.push_back({1,1,1,1});
                slaveBuffer.push_back({1,1,1,1});

                masterBufferQ->push(masertBuffer);
                slaveBufferQ->push(slaveBuffer);
                flowCtl.start();

                //master
                auto masterOutBuffer = resbufferQ->pop();
                VTF_LOGI << "master:";
                for (int i = 0; i < masterOutBuffer.size(); i++) {
                    std::stringstream ss;
                    ss << "[";
                    for (int j = 0; j < masterOutBuffer.at(i).size(); j++) {
                        ss << masterOutBuffer.at(i).at(j) << " "; 
                    }
                    VTF_LOGI << ss.str() << "]" ;
                }

                //slave
                VTF_LOGI << "slave:";
                auto slaveOutBuffer = resbufferQ->pop();
                for (int i = 0; i < slaveOutBuffer.size(); i++) {
                    std::stringstream ss;
                    ss << "[";
                    for (int j = 0; j < slaveOutBuffer.at(i).size(); j++) {
                        ss << slaveOutBuffer[i][j] << " ";
                    }

                    VTF_LOGI << ss.str() << "]" ;
                }
                std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(100));
            }
        },
        p1MasterbufferQ, 
        p1SlavebufferQ,
        resbufferQ
    );
    dataSourceThread.join();

}