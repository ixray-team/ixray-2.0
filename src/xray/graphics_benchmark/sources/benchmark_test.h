////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef BENCHMARK_TEST_H_INCLUDED
#define BENCHMARK_TEST_H_INCLUDED

namespace xray {
namespace graphics_benchmark {

class benchmark;
class gpu_timer;

class benchmark_test
{
public:
	virtual bool initialize	( benchmark&, char const*, std::map<std::string,std::string>&, std::string&)	{ return true; }
	virtual void execute	( benchmark&, u32, u32, gpu_timer&)												{ }
	virtual void finalize	( benchmark&)																	{ }
}; // class benchmark_test


template<class T>
class creator_base
{
public:
	static benchmark_test* creator	( ) { return NEW (T)();}
}; // class creator_base


} // namespace graphics_benchmark
} // namespace xray

#endif // #ifndef BENCHMARK_TEST_H_INCLUDED