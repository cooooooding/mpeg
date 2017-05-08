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

#ifndef __SIFILTER_HPP__
#define __SIFILTER_HPP__

#include <thread>
#include <string>
#include <vector>
#include <functional>

//using callBackType = std::function<void(unsigned char*, size_t)>;
using callBackType = std::function<void(char*, size_t)>;

class CTSfilter
{
	friend class CTSfilterManager;
public  :
    CTSfilter();
    virtual ~CTSfilter();
protected :
	unsigned int pid;
	//std::thread trId;
	std::string filterName;
	callBackType pCallBack;
};

class CTSfilterManager
{
public  :
    static CTSfilterManager* getInstance(void);
    //virtual void startReceiveBuffer(unsigned char *buffer, size_t num);
    virtual void startReceiveBuffer(char *buffer, size_t num);
    virtual void requestSectionFilter(std::string filterName, unsigned int pid, callBackType pCallBack);
    virtual void closeRequestSectionFilter(unsigned int pid);
    virtual void closeTSFilterMgr(void);
protected :
	std::vector<CTSfilter*> filterMgr;
private :
	static CTSfilterManager *inst;
    CTSfilterManager();
    virtual ~CTSfilterManager();
};
#endif