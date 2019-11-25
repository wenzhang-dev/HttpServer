#ifndef code_noncopyable_h
#define code_noncopyable_h

namespace webserver
{
	
class noncopyable
{
public:
	noncopyable(const noncopyable &) = delete;
	void operator=(const noncopyable &) = delete;
	
protected:
	/* default dtor and ctor is fine */
	noncopyable() = default;
	~noncopyable() = default;
};
	
}

#endif
