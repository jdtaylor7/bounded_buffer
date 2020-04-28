#ifndef BOUNDED_BUFFER_HPP
#define BOUNDED_BUFFER_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class BoundedBuffer
{
public:
    explicit BoundedBuffer(std::size_t cap_) : cap(cap_) {}

    BoundedBuffer(std::size_t cap_, T empty_value_) :
        cap(cap_), empty_value(empty_value_) {}

    BoundedBuffer(std::size_t cap_,
                  T empty_value_,
                  std::chrono::milliseconds timeout_) :
        cap(cap_),
        empty_value(empty_value_),
        timeout(timeout_)
    {}

    bool empty() const;
    std::size_t size() const;
    std::size_t capacity() const;
    std::size_t dropped_elements() const;

    T front() const;
    T back() const;

    void push(const T&);
    T pop();
private:
    std::queue<T> q;

    std::mutex m;
    std::condition_variable q_has_element;
    std::condition_variable q_has_space;

    std::size_t cap;
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero();

    std::size_t dropped{};
    T empty_value{};
};

template <typename T>
bool BoundedBuffer<T>::empty() const
{
    return q.empty();
}

template <typename T>
std::size_t BoundedBuffer<T>::size() const
{
    return q.size();
}

template <typename T>
std::size_t BoundedBuffer<T>::capacity() const
{
    return cap;
}

template <typename T>
std::size_t BoundedBuffer<T>::dropped_elements() const
{
    return dropped;
}

template <typename T>
T BoundedBuffer<T>::front() const
{
    return q.front();
}

template <typename T>
T BoundedBuffer<T>::back() const
{
    return q.back();
}

template <typename T>
void BoundedBuffer<T>::push(const T& e)
{
    std::unique_lock<std::mutex> lk(m);
    if (q_has_space.wait_for(lk,
                             timeout,
                             [this]{ return q.size() != capacity(); }))
    {
        q.push(e);
    }
    else
    {
        dropped++;
    }

    q_has_element.notify_one();
}

template <typename T>
T BoundedBuffer<T>::pop()
{
    T rv{};
    std::unique_lock<std::mutex> lk(m);
    if (q_has_element.wait_for(lk, timeout, [this]{ return !q.empty(); }))
    {
        rv = q.front();
        q.pop();
    }
    else
    {
        rv = empty_value;
    }

    q_has_space.notify_one();
    return rv;
}

#endif /* BOUNDED_BUFFER_HPP */
