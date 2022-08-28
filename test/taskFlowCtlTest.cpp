/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-05-29 16:13:40
 */
#include "../src/core/task/task_flow_ctl.hpp"
#include "../src/core/utils/queue/blocking_queue.hpp"
#include "../src/core/utils/time_util.hpp"
#include <chrono>

using Buffer = std::vector<std::vector<int>>;
using BufferQ = vtf::utils::queue::BlockingQueue<Buffer>;

int main() {
  vtf::task::TaskFlowCtl flowCtl(true);

  std::shared_ptr<BufferQ> p1MasterbufferQ = std::make_shared<BufferQ>(8);
  std::shared_ptr<BufferQ> p1SlavebufferQ = std::make_shared<BufferQ>(8);
  std::shared_ptr<BufferQ> P2MasterbufferQ = std::make_shared<BufferQ>(8);
  std::shared_ptr<BufferQ> P2SlavebufferQ = std::make_shared<BufferQ>(8);
  std::shared_ptr<BufferQ> mdp1bufferQ = std::make_shared<BufferQ>(8);

  std::shared_ptr<BufferQ> resbufferQ = std::make_shared<BufferQ>(8);

  auto P1NodeMaster = flowCtl.addTaskWithTaskInfo(
      {vtf::task::TaskPriority::NORMAL, "P1NodeMaster"},
      [](std::shared_ptr<BufferQ> inbufferQ,
         std::shared_ptr<BufferQ> outBufferQ) {
        auto buffer = inbufferQ->pop();

        for (int col = 0; col < 4; col++) {
          buffer[0][col] *= 2;
        }
        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(200));
        outBufferQ->push(buffer);
      },
      p1MasterbufferQ, P2MasterbufferQ);

  auto P1NodeSlave = flowCtl.addTaskWithTaskInfo(
      {vtf::task::TaskPriority::NORMAL, "P1NodeSlave"},
      [](std::shared_ptr<BufferQ> inbufferQ,
         std::shared_ptr<BufferQ> outBufferQ) {
        auto buffer = inbufferQ->pop();
        for (int col = 0; col < 4; col++) {
          buffer[0][col] *= 2;
        }
        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(200));
        outBufferQ->push(buffer);
      },
      p1SlavebufferQ, P2SlavebufferQ);

  auto P2SMasterNode = flowCtl.addTaskWithTaskInfo(
      {vtf::task::TaskPriority::NORMAL, "P2SMasterNode"},
      [](std::shared_ptr<BufferQ> inbufferQ,
         std::shared_ptr<BufferQ> outBufferQ) {
        auto buffer = inbufferQ->pop();
        for (int col = 0; col < 4; col++) {
          buffer[0][col] *= 2;
        }
        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(200));
        outBufferQ->push(buffer);
      },
      P2MasterbufferQ, mdp1bufferQ);

  auto P2SSlaveNode = flowCtl.addTaskWithTaskInfo(
      {vtf::task::TaskPriority::NORMAL, "P2SSlaveNode"},
      [](std::shared_ptr<BufferQ> inbufferQ,
         std::shared_ptr<BufferQ> outBufferQ) {
        auto buffer = inbufferQ->pop();
        for (int col = 0; col < 4; col++) {
          buffer[0][col] *= 2;
        }
        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(200));
        outBufferQ->push(buffer);
      },
      P2SlavebufferQ, mdp1bufferQ);

  auto mdpNode1 = flowCtl.addTaskWithTaskInfo(
      {vtf::task::TaskPriority::NORMAL, "mdpNode1"},
      [](std::shared_ptr<BufferQ> inbufferQ,
         std::shared_ptr<BufferQ> outBufferQ) {
        while (!inbufferQ->isEmpty()) {
          auto buffer = inbufferQ->pop();
          for (int col = 0; col < 4; col++) {
            buffer[0][col] *= 2;
          }
          outBufferQ->push(buffer);
          std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(200));
        }
      },
      mdp1bufferQ, resbufferQ);

  P1NodeMaster->connect(P2SMasterNode);
  P1NodeSlave->connect(P2SSlaveNode);
  P2SMasterNode->connect(mdpNode1);
  P2SSlaveNode->connect(mdpNode1);

  std::thread dataSourceThread(
      [&flowCtl](std::shared_ptr<BufferQ> masterBufferQ,
                 std::shared_ptr<BufferQ> slaveBufferQ,
                 std::shared_ptr<BufferQ> resbufferQ) {
        for (int i = 0; i < 10; i++) {
          Buffer masertBuffer;
          masertBuffer.push_back({1, 1, 1, 1});
          masertBuffer.push_back({1, 1, 1, 1});
          masertBuffer.push_back({1, 1, 1, 1});
          masertBuffer.push_back({1, 1, 1, 1});

          Buffer slaveBuffer;
          slaveBuffer.push_back({1, 1, 1, 1});
          slaveBuffer.push_back({1, 1, 1, 1});
          slaveBuffer.push_back({1, 1, 1, 1});
          slaveBuffer.push_back({1, 1, 1, 1});

          masterBufferQ->push(masertBuffer);
          slaveBufferQ->push(slaveBuffer);
          flowCtl.start();

          // master
          auto masterOutBuffer = resbufferQ->pop();
          VTF_LOGI("master");
          for (size_t i = 0; i < masterOutBuffer.size(); i++) {
            std::stringstream ss;
            ss << "[";
            for (size_t j = 0; j < masterOutBuffer.at(i).size(); j++) {
              ss << masterOutBuffer.at(i).at(j) << " ";
            }
            VTF_LOGE("{0}]", ss.str());
          }

          // slave
          VTF_LOGI("slave");
          auto slaveOutBuffer = resbufferQ->pop();
          for (size_t i = 0; i < slaveOutBuffer.size(); i++) {
            std::stringstream ss;
            ss << "[";
            for (int j = 0; j < slaveOutBuffer.at(i).size(); j++) {
              ss << slaveOutBuffer[i][j] << " ";
            }

            VTF_LOGE("{0}]", ss.str());
          }
          std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(100));
        }
      },
      p1MasterbufferQ, p1SlavebufferQ, resbufferQ);
  dataSourceThread.join();
}