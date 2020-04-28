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
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 1);
    EXPECT_EQ(buf->front(), 2);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 4);
    EXPECT_FALSE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 2);

    result = buf->pop_wait();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 2);
    EXPECT_EQ(buf->front(), 3);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 3);

    result = buf->pop_wait_for();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 3);
    EXPECT_EQ(buf->front(), 4);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 2);

    result = buf->try_pop();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 4);
    EXPECT_EQ(buf->front(), 5);
    EXPECT_EQ(buf->back(), 5);
    EXPECT_EQ(buf->size(), 1);

    result = buf->try_pop();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 5);
    EXPECT_EQ(buf->size(), 0);
    EXPECT_TRUE(buf->empty());
    EXPECT_EQ(buf->dropped_elements(), 1);

    // EXPECT_EQ(buf->pop(), nullptr);
}

// /*
//  * Pop value from full buffer during push operation.
//  */
// TEST(BoundedBufferTests, UnstickPushTest)
// {
//     using namespace std::chrono_literals;
//
//     std::size_t capacity = 5;
//     auto timeout = 4s;
//
//     auto buf = std::make_shared<BoundedBuffer<int>>(capacity, timeout);
//
//     buf->push(1);
//     buf->push(2);
//     buf->push(3);
//     buf->push(4);
//     buf->push(5);
//
//     std::thread t([buf]{
//         std::this_thread::sleep_for(1s);
//         EXPECT_EQ(buf->pop(), 1);
//     });
//     t.detach();
//
//     buf->push(6);
//
//     EXPECT_EQ(buf->front(), 2);
//     EXPECT_EQ(buf->back(), 6);
//     EXPECT_EQ(buf->size(), 5);
//     EXPECT_EQ(buf->dropped_elements(), 0);
// }
//
// /*
//  * Push value into empty buffer during pop operation.
//  */
// TEST(BoundedBufferTests, UnstickPopTest)
// {
//     using namespace std::chrono_literals;
//
//     std::size_t capacity = 5;
//     auto timeout = 4s;
//
//     auto buf = std::make_shared<BoundedBuffer<int>>(capacity, timeout);
//
//     std::thread t([buf]{
//         std::this_thread::sleep_for(1s);
//         buf->push(3);
//     });
//     t.detach();
//
//     EXPECT_EQ(buf->pop(), 3);
//
//     EXPECT_EQ(buf->size(), 0);
// }

// /*
//  * Ensure correctness under load.
//  */
// TEST(BoundedBufferTests, ThoughputTest)
// {
//     using namespace std::chrono_literals;
//
//     std::size_t buf_cap = 1024;
//
//     std::size_t arr_cap = 1'000'000;
//
//     auto buf = std::make_shared<BoundedBuffer<int>>(buf_cap);
//
//     std::vector<int> producer(arr_cap, 0);
//     std::vector<int> consumer{};
//
//     for (std::size_t i = 0; i < producer.size(); i++)
//         producer[i] = i;
//
//     std::thread t1([buf, producer]{
//         for (const auto& e : producer)
//             buf->push_wait(e);
//     });
//
//     std::thread t2([buf, &consumer, arr_cap]{
//         std::this_thread::sleep_for(1s);
//         while (consumer.size() < arr_cap)
//             consumer.push_back(*(buf->pop_wait()));
//     });
//
//     t1.join();
//     t2.join();
//
//     EXPECT_EQ(producer.size(), consumer.size());
//     EXPECT_EQ(consumer.size(), arr_cap);
//
//     for (std::size_t i = 0; i < consumer.size(); i++)
//         EXPECT_EQ(consumer[i], i);
// }
//
// /*
//  * Ensure correctness under load for methods with timeout functionality.
//  */
// TEST(BoundedBufferTests, ThoughputTestTimeout)
// {
//     using namespace std::chrono_literals;
//
//     std::size_t buf_cap = 1024;
//     auto timeout = 10s;
//
//     std::size_t arr_cap = 1'000'000;
//
//     auto buf = std::make_shared<BoundedBuffer<int>>(buf_cap, timeout);
//
//     std::vector<int> producer(arr_cap, 0);
//     std::vector<int> consumer{};
//
//     for (std::size_t i = 0; i < producer.size(); i++)
//         producer[i] = i;
//
//     std::thread t1([buf, producer]{
//         for (const auto& e : producer)
//             EXPECT_TRUE(buf->push_wait_for(e));
//     });
//
//     std::thread t2([buf, &consumer, arr_cap]{
//         std::this_thread::sleep_for(1s);
//         while (consumer.size() < arr_cap)
//             consumer.push_back(*(buf->pop_wait_for()));
//     });
//
//     t1.join();
//     t2.join();
//
//     EXPECT_EQ(producer.size(), consumer.size());
//     EXPECT_EQ(consumer.size(), arr_cap);
//
//     for (std::size_t i = 0; i < consumer.size(); i++)
//         EXPECT_EQ(consumer[i], i);
// }
