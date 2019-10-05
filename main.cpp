#include "ThreadPool.h"
#include <iostream>

int add(int a, int b)
{
    return a+b;
}

int main() {
    using namespace mslite;
    auto threadPool = ThreadPool::GetThreadPool(2);
    threadPool->Run();
    std::future<int> res[10000];
    for(int i=0;i<10000;i++)
        res[i]=threadPool->enqueue(add,1,2);
    for(int i=0;i<10000;i++)
        res[i].wait();
    for(int i=0;i<10000;i++)
        std::cout<<res[i].get()<<std::endl;

}
