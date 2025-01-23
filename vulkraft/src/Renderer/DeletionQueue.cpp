#include <Renderer/DeletionQueue.h>

void DeletionQueue::Flush(void)
{
    int i;

    for(i=this->queue.size()-1; i>=0; i--)
        this->queue[i]();
    
    queue.clear();
}

void DeletionQueue::Push(std::function<void(void)>&& fun)
{
    this->queue.push_back(fun);
}