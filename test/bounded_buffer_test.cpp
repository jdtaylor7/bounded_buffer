#include <iostream>
#include <memory>

#include "gtest/gtest.h"

#include "src/bounded_buffer.hpp"

TEST(BoundedBufferTests, BasicApiTest)
{
    using namespace std::chrono_literals;

    std::size_t capacity = 5;
    auto buf = std::make_shared<BoundedBuffer<int>>(capacity, 5000ms);

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

    // Space not made within 5 seconds, so 6 not pushed.
    buf->push(6);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
}
