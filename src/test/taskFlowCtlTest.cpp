/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-16 21:56:43
 */
#include "../core/taskflowctl.hpp"
#include "../core/utils.hpp"
#include <chrono>



using Buffer = std::vector<std::vector<int>>;
using BufferQ = std::queue<Buffer>;

int main()
{
    vtf::TaskFlowCtl flowCtl(true);

    std::shared_ptr<BufferQ> p1MasterbufferQ = std::make_shared<BufferQ>();
    std::shared_ptr<BufferQ> p1SlavebufferQ = std::make_shared<BufferQ>();
    std::shared_ptr<BufferQ> P2MasterbufferQ = std::make_shared<BufferQ>();
    std::shared_ptr<BufferQ> P2SlavebufferQ = std::make_shared<BufferQ>();
    std::shared_ptr<BufferQ> mdp1bufferQ = std::make_shared<BufferQ>();
    std::shared_ptr<BufferQ> mdp2bufferQ = std::make_shared<BufferQ>();

    std::shared_ptr<BufferQ> resbufferQ = std::make_shared<BufferQ>();



    auto P1NodeMaster = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P1NodeMaster"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->front();
            
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
            inbufferQ->pop();
        },
        p1MasterbufferQ,
        P2MasterbufferQ
    );

    auto P1NodeSlave = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"P1NodeSlave"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            auto buffer = inbufferQ->front();
            inbufferQ->pop();
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
            std::cout << inbufferQ->size();
            auto buffer = inbufferQ->front();
            inbufferQ->pop();
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
            auto buffer = inbufferQ->front();
            inbufferQ->pop();
            for (int col = 0; col < 4; col++) {
                buffer[0][col] *= 2;
            }
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            outBufferQ->push(buffer);
        },
        P2SlavebufferQ,
        mdp2bufferQ
    );

    auto mdpNode1 = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"mdpNode1"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            // while (!inbufferQ->empty()) {
                auto buffer = inbufferQ->front();
                std::cout << "mdp1: buffer.size()" << buffer.size() << std::endl;
                for (int col = 0; col <4; col++) {
                    buffer[0][col] *= 2;
                }
                outBufferQ->push(buffer);
                inbufferQ->pop();
                std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));

            // }
        },
        mdp1bufferQ,
        resbufferQ
    );

    auto mdpNode2 = flowCtl.addTaskWithTaskInfo(
        {vtf::TaskPriority::NORMAL,"mdpNode2"}, 
        [](std::shared_ptr<BufferQ> inbufferQ, std::shared_ptr<BufferQ> outBufferQ) {
            // while (!inbufferQ->empty()) {
                auto buffer = inbufferQ->front();
                std::cout << "mdp2: buffer.size()" << buffer.size() << std::endl;
                for (int col = 0; col <4; col++) {
                    buffer[0][col] *= 2;
                }
                outBufferQ->push(buffer);
                inbufferQ->pop();
                std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
            // }
        },
        mdp2bufferQ,
        resbufferQ
    );


    P1NodeMaster->precede(P2SMasterNode);
    P1NodeSlave->precede(P2SSlaveNode);
    P2SMasterNode->precede(mdpNode1);
    P2SSlaveNode->precede(mdpNode2);

    // auto start = std::chrono::system_clock::now();
    // flowCtl.start();
    // auto end = std::chrono::system_clock::now();
    // std::cout << "all task need: " << vtf::util::TimeUtil::convertTime<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // std::cout << "out buffer content: ";

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
                auto masterOutBuffer = resbufferQ->front();
                resbufferQ->pop();
                std::cout << "master:\n";
                for (int i = 0; i < masterOutBuffer.size(); i++) {
                    std::cout << "[";
                    for (int j = 0; j < masterOutBuffer.at(i).size(); j++) {
                        std::cout << masterOutBuffer.at(i).at(j) << " "; 
                    }
                    std::cout << "]" << std::endl;
                }

                //slave
                std::cout << "slave:\n";
                auto slaveOutBuffer = resbufferQ->front();
                resbufferQ->pop();
                for (int i = 0; i < slaveOutBuffer.size(); i++) {
                    std::cout << "[";
                    for (int j = 0; j < slaveOutBuffer.at(i).size(); j++) {
                        std::cout << slaveOutBuffer.at(i).at(j) << " "; 
                    }
                    std::cout << "]" << std::endl;
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