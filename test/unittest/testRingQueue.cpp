#include "../../src/core/utils/queue/ring_queue.hpp"
#include "testUtils.hpp"



int main()
{
    vtf::RingQueue<int> rq(4);
    rq.push(1);
    rq.push(2);
    rq.push(3);
    rq.push(4);
    ASSERT_EQ(4, (int)rq.size());
    ASSERT_EQ(false, rq.empty());
    ASSERT_EQ(true, rq.full());
    ASSERT_EQ(4, rq.real());
    ASSERT_EQ(1, rq.front());
    rq.push(5);
    ASSERT_EQ(4, rq.real());
    ASSERT_EQ(1, rq.front());

    rq.pop();
    ASSERT_EQ(4, rq.real());
    ASSERT_EQ(2, rq.front());
    ASSERT_EQ(3, (int)rq.size());

    rq.push(5);
    ASSERT_EQ(5, rq.real());
    ASSERT_EQ(2, rq.front());
    ASSERT_EQ(4, (int)rq.size());

    rq.pop();
    ASSERT_EQ(3, (int)rq.size());
    ASSERT_EQ(5, rq.real());
    ASSERT_EQ(3, rq.front());

    rq.pop();
    ASSERT_EQ(2, (int)rq.size());
    ASSERT_EQ(5, rq.real());
    ASSERT_EQ(4, rq.front());
    rq.pop();
    ASSERT_EQ(1, (int)rq.size());
    ASSERT_EQ(5, rq.real());
    ASSERT_EQ(5, rq.front());
    rq.pop();
    ASSERT_EQ(0, (int)rq.size());
    ASSERT_EQ(true, rq.empty());
    ASSERT_EQ(false, rq.full());

    PRINT_RESULT();

    return 0;
}
