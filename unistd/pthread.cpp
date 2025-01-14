
#include "pthread.h"

#include "../portable/Logger.h"

#include <thread>
#include <system_error>

struct pthread_attr
{	bool isJoinable;
public:
	pthread_attr()
	:	isJoinable(true)
	{}
};
typedef struct pthread_attr pthread_attr_t;

struct PortableThread
:	public std::thread
{	
public:
	pthread_attr_t attr;
	PortableThread(const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg)
	:	std::thread(start_routine,arg)
	{	if(attr)
		{	this->attr = *attr;
	}	}
};

int pthread_setschedparam(pthread_t pthread, int policy, const sched_param* param)
{	if(!pthread)
	{	return -1;
	}
	HANDLE h = (HANDLE) pthread->native_handle();
	return SetThreadPriority(h, param->sched_priority)? 0:-1;
}

#ifdef VERBOSE_PTHREAD
int uni_pthread_create(pthread_t* pthread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg,const char* name)
{	SysLogMsg("Thread",name); 
	PortableThread* t=new PortableThread(attr,start_routine,arg);
	*pthread=t;
	if(!t->attr.isJoinable)
	{	t->detach();
	}
	return 0;
}

#define pthread_create(t,at,f,arg) uni_pthread_create(t,at,f,arg,#f)
#else
int pthread_create(pthread_t* pthread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg)
{	PortableThread* t=0;
    try 
	{	t = new PortableThread(attr,start_routine,arg);
    } 
	catch(const std::system_error& e) 
	{	printf("Caught system_error %s\n",e.what());
		return -1;
    }
	*pthread=t;
	if(!t->attr.isJoinable)
	{	t->detach();
	}
	return 0;
}
#endif

int pthread_attr_setdetachstate(pthread_attr_t *attr, ThreadState detachstate)
{	if(!attr)
	{	return -1;
	}
	attr->isJoinable = (detachstate == PTHREAD_CREATE_JOINABLE);
	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{	if(!attr)
	{	return -1;
	}
	if(attr->isJoinable)
	{	*detachstate = (int) PTHREAD_CREATE_JOINABLE;
		return true;
	}
	*detachstate = (int) PTHREAD_CREATE_DETACHED;
	return 0;
}

int pthread_join(pthread_t thread, void **retval)
{	if(!thread)
	{	return -1;
	}
	if(!thread->attr.isJoinable)
	{	return -1;
	}
    try 
	{	thread->join();
    } 
	catch(const std::system_error& e) 
	{	printf("Caught system_error %s\n",e.what());
		return -1;
    }
	return 0;
}

int pthread_detach(pthread_t thread)
{	thread->detach();
	return 0;
}