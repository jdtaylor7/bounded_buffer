#include <iostream>
#include <memory>

#include "gtest/gtest.h"

#include "src/bounded_buffer.hpp"

/*
 * Test basic all member functions under simple conditions. Does not test with
 * any multithreading.
 */
TEST(BoundedBufferTests, BasicFuncTest)
{
    using namespace std::chrono_literals;

    std::size_t capacity = 5;
    int empty_value = std::numeric_limits<int>::min();
    auto timeout = 2s;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity,
        empty_value, timeout);

    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_TRUE(buf->empty());

    buf->push(1);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 1);
    EXPECT_EQ(buf->size(), 1);
    EXPECT_FALSE(buf->empty());

    buf->push(2);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 2);
    EXPECT_EQ(buf->size(), 2);

    buf->push(3);
    buf->push(4);
    buf->push(5);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->dropped_elements(), 0);

    // Space not made within 2 seconds, so 6 not pushed.
    buf->push(6);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->dropped_elements(), 1);

    EXPECT_EQ(buf->pop(), 1);
    EXPECT_EQ(buf->front(), 2);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 4);
    EXPECT_FALSE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 1);

    EXPECT_EQ(buf->pop(), 2);
    EXPECT_EQ(buf->front(), 3);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 3);

    EXPECT_EQ(buf->pop(), 3);
    EXPECT_EQ(buf->front(), 4);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 2);

    EXPECT_EQ(buf->pop(), 4);
    EXPECT_EQ(buf->front(), 5);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 1);

    EXPECT_EQ(buf->pop(), 5);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_TRUE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 1);

    EXPECT_EQ(buf->pop(), empty_value);
}

void unstick_push(std::shared_ptr<BoundedBuffer> buf)
{
    buf->pop();
}

/*
 * Pop value from full buffer during push operation.
 */
TEST(BoundedBufferTests, UnstickPushTest)
{
    using namespace std::chrono_literals;

    std::size_t capacity = 5;
    int empty_value = std::numeric_limits<int>::min();
    auto timeout = 4s;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity,
        empty_value, timeout);

    buf->push(1);
    buf->push(2);
    buf->push(3);
    buf->push(4);
    buf->push(5);

    buf->push(6);
}

/*
 * Push value into empty buffer during pop operation.
 */
TEST(BoundedBufferTests, UnstickPopTest)
{
}

/*
 * Ensure correctness under load.
 */
TEST(BoundedBufferTests, ThoughputTest)
{
}
