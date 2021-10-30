/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 15:59:50
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 16:26:01
 */
#include "../core/pipeline/P1Node.hpp"
#include "../core/pipeline/P2Node.hpp"
#include <thread>


int main()
{
    std::shared_ptr<vtf::pipeline::P1Node> p1Node = std::make_shared<vtf::pipeline::P1Node>();
    std::shared_ptr<vtf::pipeline::P2Node> p2Node = std::make_shared<vtf::pipeline::P2Node>();



    p1Node->connect(p2Node);

    while (true) {

    }
}