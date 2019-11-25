#ifndef code_CurrentThread_h
#define code_CurrentThread_h

#include "macros.h"

namespace webserver
{

namespace CurrentThread
{

//internel
//may be able to use non-POD types
extern thread_local int t_cachedTid;
extern thread_local char t_tidString[32];
extern thread_local int t_tidStringLength;
extern thread_local const char *t_threadName;

void cacheTid();

inline int tid()
{
	if(unlikely(t_cachedTid == 0))
	{
		cacheTid();
	}
	return t_cachedTid;
}

inline const char *tidString()
{
	return t_tidString;	
}

inline int tidStringLength()
{
	return t_tidStringLength;
}

inline const char *name()
{
	return t_threadName;
}

} //namespace CurrentThread

} //namespace webserver

#endif
