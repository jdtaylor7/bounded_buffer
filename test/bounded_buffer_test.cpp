#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

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
    auto timeout = 2s;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity, timeout);

    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_TRUE(buf->empty());

    EXPECT_TRUE(buf->try_push(1));
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 1);
    EXPECT_EQ(buf->size(), 1);
    EXPECT_FALSE(buf->empty());

    buf->push_wait(2);
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 2);
    EXPECT_EQ(buf->size(), 2);

    EXPECT_TRUE(buf->push_wait_for(3));
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 3);
    EXPECT_EQ(buf->size(), 3);

    EXPECT_TRUE(buf->try_push(4));

    EXPECT_TRUE(buf->try_push(5));
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->dropped_elements(), 0);

    // Space not available, push fails.
    EXPECT_FALSE(buf->try_push(6));
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->dropped_elements(), 1);

    // Space not made available within 2 seconds, push fails.
    EXPECT_FALSE(buf->push_wait_for(7));
    EXPECT_EQ(buf->front(), 1);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->dropped_elements(), 2);

    std::shared_ptr<int> result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 1);
    EXPECT_EQ(buf->front(), 2);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 4);
    EXPECT_FALSE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 2);

    result = buf->pop_wait();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 2);
    EXPECT_EQ(buf->front(), 3);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 3);

    result = buf->pop_wait_for();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 3);
    EXPECT_EQ(buf->front(), 4);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 2);

    result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 4);
    EXPECT_EQ(buf->front(), 5);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 1);

    result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 5);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_TRUE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 2);

    result = buf->try_pop();
    EXPECT_EQ(result, nullptr);

    result = buf->pop_wait_for();
    EXPECT_EQ(result, nullptr);
}

/*
 * Pop value from full buffer while push operation is waiting.
 */
TEST(BoundedBufferTests, UnstickPushTest)
{
    using namespace std::chrono_literals;

    std::size_t capacity = 5;
    auto timeout = 4s;
    auto max_test_time = 2s;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity, timeout);

    EXPECT_TRUE(buf->try_push(1));
    EXPECT_TRUE(buf->try_push(2));
    EXPECT_TRUE(buf->try_push(3));
    EXPECT_TRUE(buf->try_push(4));
    EXPECT_TRUE(buf->try_push(5));

    auto time1 = std::chrono::steady_clock::now();

    std::thread t([buf]{
        std::this_thread::sleep_for(1s);
        auto result = buf->try_pop();
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(*result, 1);
    });
    t.detach();

    EXPECT_TRUE(buf->push_wait_for(6));

    auto time2 = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1);

    EXPECT_LT(elapsed, max_test_time);

    EXPECT_EQ(buf->front(), 2);
    EXPECT_EQ(buf->back(), 6);
    EXPECT_EQ(buf->size(), 5);
    EXPECT_EQ(buf->dropped_elements(), 0);
}

/*
 * Push value into empty buffer during pop operation.
 */
TEST(BoundedBufferTests, UnstickPopTest)
{
    using namespace std::chrono_literals;

    std::size_t capacity = 5;
    auto timeout = 4s;
    auto max_test_time = 3s;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity, timeout);

    auto time1 = std::chrono::steady_clock::now();

    std::thread t([buf]{
        std::this_thread::sleep_for(2s);
        EXPECT_TRUE(buf->try_push(3));
    });
    t.detach();

    auto result = buf->pop_wait_for();

    auto time2 = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1);

    EXPECT_LT(elapsed, max_test_time);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 3);
    EXPECT_EQ(buf->size(), 0);
}

/*
 * Ensure correctness under load.
 */
TEST(BoundedBufferTests, ThoughputTest)
{
    using namespace std::chrono_literals;

    std::size_t buf_cap = 1024;

    std::size_t arr_cap = 1'000'000;

    auto buf = std::make_shared<BoundedBuffer<int>>(buf_cap);

    std::vector<int> producer(arr_cap, 0);
    std::vector<int> consumer{};

    for (std::size_t i = 0; i < producer.size(); i++)
        producer[i] = i;

    std::thread t1([buf, producer]{
        for (const auto& e : producer)
            buf->push_wait(e);
    });

    std::thread t2([buf, &consumer, arr_cap]{
        std::this_thread::sleep_for(1s);
        while (consumer.size() < arr_cap)
            consumer.push_back(*(buf->pop_wait()));
    });

    t1.join();
    t2.join();

    EXPECT_EQ(producer.size(), consumer.size());
    EXPECT_EQ(consumer.size(), arr_cap);

    for (std::size_t i = 0; i < consumer.size(); i++)
        EXPECT_EQ(consumer[i], i);
}

/*
 * Ensure correctness under load for methods with timeout functionality.
 */
TEST(BoundedBufferTests, ThoughputTestTimeout)
{
    using namespace std::chrono_literals;

    std::size_t buf_cap = 1024;
    auto timeout = 10s;

    std::size_t arr_cap = 1'000'000;

    auto buf = std::make_shared<BoundedBuffer<int>>(buf_cap, timeout);

    std::vector<int> producer(arr_cap, 0);
    std::vector<int> consumer{};

    for (std::size_t i = 0; i < producer.size(); i++)
        producer[i] = i;

    std::thread t1([buf, producer]{
        for (const auto& e : producer)
            EXPECT_TRUE(buf->push_wait_for(e));
    });

    std::thread t2([buf, &consumer, arr_cap]{
        std::this_thread::sleep_for(1s);
        while (consumer.size() < arr_cap)
            consumer.push_back(*(buf->pop_wait_for()));
    });

    t1.join();
    t2.join();

    EXPECT_EQ(producer.size(), consumer.size());
    EXPECT_EQ(consumer.size(), arr_cap);

    for (std::size_t i = 0; i < consumer.size(); i++)
        EXPECT_EQ(consumer[i], i);
}

/*
 * Test BoundedBuffer::force_push.
 */
TEST(BoundedBufferTests, ForcePushTest)
{
    std::size_t capacity = 3;

    auto buf = std::make_shared<BoundedBuffer<int>>(capacity);

    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->size(), 0);

    EXPECT_TRUE(buf->try_push(1));
    EXPECT_TRUE(buf->try_push(2));
    EXPECT_TRUE(buf->try_push(3));

    buf->force_push(4);
    EXPECT_EQ(buf->capacity(), capacity);
    EXPECT_EQ(buf->size(), 3);
    EXPECT_EQ(buf->front(), 2);
    EXPECT_EQ(buf->back(), 4);
    EXPECT_EQ(buf->dropped_elements(), 0);

    std::shared_ptr<int> result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 2);
    EXPECT_EQ(buf->front(), 3);
    EXPECT_EQ(buf->back(), 4);
    EXPECT_EQ(buf->size(), 2);

    result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 3);
    EXPECT_EQ(buf->front(), 4);
    EXPECT_EQ(buf->back(), 4);
    EXPECT_EQ(buf->size(), 1);

    result = buf->try_pop();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 4);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_EQ(buf->empty(), true);

    result = buf->try_pop();
    ASSERT_EQ(result, nullptr);
}
