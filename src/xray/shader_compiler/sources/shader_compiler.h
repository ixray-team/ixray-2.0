////////////////////////////////////////////////////////////////////////////
//	Created		: 18.05.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef SHADER_COMPILER_H_INCLUDED
#define SHADER_COMPILER_H_INCLUDED

#include "define_manager.h"

namespace xray {
namespace shader_compiler {


struct compiler : private boost::noncopyable 
{
	compiler(
		std::string const& file_name,
		char const* buffer, 
		s32 buffer_size, 
		char const* main_function_id,
		std::vector<define_set>& in_defines_set):
			m_file_name(file_name),
			m_defines_set(in_defines_set),
			m_buffer(buffer),
			m_buffer_size(buffer_size),
			m_main_function_id(main_function_id)
	{
	}
	
	bool compile ( bool legacy, u32& out_shader_size, u32& out_instruction_number, std::vector<std::string>& out_errors, std::vector<std::string>& out_warnings );
	
private:
	std::string				 m_file_name;
	const char*				 m_buffer;
	s32						 m_buffer_size;
	std::string				 m_main_function_id;
	std::vector<define_set>& m_defines_set;
}; // class compiler

} // namespace shader_compiler
} // namespace xray

#endif // #ifndef SHADER_COMPILER_H_INCLUDED