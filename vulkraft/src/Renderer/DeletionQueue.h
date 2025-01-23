#pragma once

#include <functional>
#include <deque>

class DeletionQueue
{
private:
    std::deque<std::function<void(void)>> queue;
public:
    void Push(std::function<void(void)>&& fun);
    void Flush(void);
};