/**
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.
In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <http://unlicense.org>
*/

#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <chrono>
#include <pthread.h>

enum class DebugLevel4MessageQueuePool
{
    DEBUG_LEVEL_NORMAL,     // No Debug Message from class
    DEBUG_LEVEL_ERROR,      // Print Error Message
    DEBUG_LEVEL_DEBUG,      // Print Debug Related Message
    DEBUG_LEVEL_TRACE,      // All Debug Message Trace including class member function
    DEBUG_LEVEL_MAX
};

static DebugLevel4MessageQueuePool gDebugLevel = DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE;

#define DEBUG_PRINT(debugPrint, debugMsg)\
{\
    if((debugPrint) <= gDebugLevel)\
    {\
        switch(debugPrint) \
        {\
            case DebugLevel4MessageQueuePool::DEBUG_LEVEL_NORMAL:\
                std::printf("[%s,%d] %s \n", __FUNCTION__, __LINE__, debugMsg);\
            break;\
            case DebugLevel4MessageQueuePool::DEBUG_LEVEL_ERROR:\
                std::printf("\033[0;31m Error in [%s,%d] %s \n", __FUNCTION__, __LINE__, debugMsg);\
            break;\
            case DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG:\
                std::printf("\033[1;32m Debug in [%s,%d] %s \n", __FUNCTION__, __LINE__, debugMsg);\
            break;\
            case DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE:\
                std::printf("\033[1;33m Trace in [%s,%d] %s \n", __FUNCTION__, __LINE__, debugMsg);\
            break;\
            default :\
            break;\
        }\
        std::printf("\033[0m");\
    }\
}

template <typename T>
class CMessageQueuePool;

template <typename T>
class CMessageQueue
{
    friend class CMessageQueuePool<T>;
private :
    std::queue<T> msgQ;
    std::mutex mtx;
    std::string nameOfmsgQ;
    pthread_mutex_t mutexForMsgQCond;
    pthread_cond_t msgQCond;    
public  :
    CMessageQueue();
    virtual ~CMessageQueue();
    virtual void sendMessage(const T& msg);
    virtual T receiveMessage(void);
    virtual bool receiveMessage(T& msg);
    virtual bool isMessageQueueEmpty(void);
};

template <typename T>
CMessageQueue<T>::CMessageQueue()
{
    msgQCond            = PTHREAD_COND_INITIALIZER;
    mutexForMsgQCond    = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_cond_init(&msgQCond, NULL);
    pthread_mutex_init(&mutexForMsgQCond, NULL);

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE, "");
}

template <typename T>
CMessageQueue<T>::~CMessageQueue()
{
    std::queue<T> empty;
    std::swap(empty, msgQ);

    pthread_cond_destroy(&msgQCond);
    pthread_mutex_destroy(&mutexForMsgQCond);
    
    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE, "");
}

template <typename T>
void CMessageQueue<T>::sendMessage(const T& msg)
{  
    mtx.lock();
    msgQ.push(msg);
    mtx.unlock();

    pthread_mutex_lock(&mutexForMsgQCond);
    pthread_cond_signal(&msgQCond);
    pthread_mutex_unlock(&mutexForMsgQCond);

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
}

template <typename T>
T CMessageQueue<T>::receiveMessage(void)
{
    T msg;
    
    pthread_mutex_lock(&mutexForMsgQCond);
    while(isMessageQueueEmpty() == true)
    {
        DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_NORMAL, "Message Queue Empty Condition");
        pthread_cond_wait(&msgQCond, &mutexForMsgQCond);
    }
    pthread_mutex_unlock(&mutexForMsgQCond);
    
    mtx.lock();
    msg = msgQ.front();
    msgQ.pop();
    mtx.unlock();
    
    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
    return msg;
}

template <typename T>
bool CMessageQueue<T>::receiveMessage(T& msg)
{   
    pthread_mutex_lock(&mutexForMsgQCond);
    while(isMessageQueueEmpty() == true)
    {
        DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_NORMAL, "Message Queue Empty Condition");
        pthread_cond_wait(&msgQCond, &mutexForMsgQCond);
    }
    pthread_mutex_unlock(&mutexForMsgQCond);

    mtx.lock();
    msg = msgQ.front();
    msgQ.pop();
    mtx.unlock();
    
    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
    return true;
}

template <typename T>
bool CMessageQueue<T>::isMessageQueueEmpty(void)
{  
    bool isEmpty = false;
    
    mtx.lock();
    isEmpty = msgQ.empty();
    mtx.unlock();

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
    return isEmpty;
}

template <typename T>
class CMessageQueuePool
{
private :
    std::map<std::string, CMessageQueue<T>*> poolOfmsgQ;
    //using iterType = typename std::map<std::string, CMessageQueue<T>*>;
    CMessageQueuePool(){};    
    static std::shared_ptr<CMessageQueuePool<T>> inst;
public  :
    void setCurrentDebugLevel(DebugLevel4MessageQueuePool debugLevel);
public  :
    virtual ~CMessageQueuePool();
    static std::shared_ptr<CMessageQueuePool<T>> getInstance(void);
    virtual bool createMessageQueue(std::string nameOfmsgQ);
    virtual CMessageQueue<T>* getMessageQueue(std::string nameOfmsgQ);
};

template <typename T>
std::shared_ptr<CMessageQueuePool<T>> CMessageQueuePool<T>::inst = nullptr;

template <typename T>
std::shared_ptr<CMessageQueuePool<T>> CMessageQueuePool<T>::getInstance(void)
{
    if(inst == nullptr)
    {
        inst.reset(new CMessageQueuePool<T>);
    }

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE, "");
    return inst;
}

template <typename T>
CMessageQueuePool<T>::~CMessageQueuePool()
{  
    for(auto iter = poolOfmsgQ.begin(); iter != poolOfmsgQ.end(); iter++)
    {
        delete iter->second;
    }

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_TRACE, "");
    poolOfmsgQ.clear();
}

template <typename T>
bool CMessageQueuePool<T>::createMessageQueue(std::string nameOfmsgQ)
{    
    if(poolOfmsgQ.find(nameOfmsgQ) != poolOfmsgQ.end())
    {
        DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_ERROR, "Already Message Queue Exist!!");
        return false;
    }
    
    CMessageQueue<T> *newMessageQueue   = new CMessageQueue<T>;
    newMessageQueue->nameOfmsgQ         = nameOfmsgQ;    
    poolOfmsgQ[nameOfmsgQ]              = newMessageQueue;

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
    return true;
}

template <typename T>
CMessageQueue<T>* CMessageQueuePool<T>::getMessageQueue(std::string nameOfmsgQ)
{
    for(auto iter = poolOfmsgQ.begin(); iter != poolOfmsgQ.end(); iter++)
    {
        if(iter->first.compare(nameOfmsgQ) == 0)
        {
            return iter->second;
        }
    }

    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
    return nullptr;
}

template <typename T>
void CMessageQueuePool<T>::setCurrentDebugLevel(DebugLevel4MessageQueuePool debugLevel)
{
    gDebugLevel = debugLevel;
    DEBUG_PRINT(DebugLevel4MessageQueuePool::DEBUG_LEVEL_DEBUG, "");
}