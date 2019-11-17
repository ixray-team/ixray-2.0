////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include <xray/console_command.h>
#include <xray/console_command_processor.h>

xray::console_commands::console_command* s_console_command_root = NULL;

using xray::math::float2;
using xray::math::float3;
using xray::console_commands::console_command;
using xray::console_commands::cc_delegate;
using xray::console_commands::cc_string;
using xray::console_commands::cc_bool;
using xray::console_commands::cc_float;
using xray::console_commands::cc_u32;
using xray::console_commands::cc_float2;
using xray::console_commands::cc_float3;

console_command::console_command(pcstr name)
:m_name(name), m_prev(s_console_command_root), m_next(NULL), m_need_args(false), m_serializable(false)
{
	//make it thread-safe
	if(m_prev)
		m_prev->m_next		= this;

	s_console_command_root	= this;
}

console_command::~console_command()
{
}

void console_command::status(status_str& dest) const
{
	strings::copy(dest, "unknown");
}

void console_command::info(info_str& dest) const
{
	strings::copy(dest, "(no info)");
}

void console_command::syntax(syntax_str& dest) const
{
	strings::copy(dest, "(no arguments)");
}

void console_command::on_invalid_syntax(pcstr args)
{
	syntax_str	buff; 
	syntax		(buff);
	LOG_WARNING	("Invalid syntax in call [%s %s]", name(), args);
	LOG_WARNING	("Valid arguments: %s", buff);
}

//--delegate
cc_delegate::cc_delegate(pcstr name, Delegate const& func, bool need_args)
:super(name), 
m_func(func)
{
	m_need_args = need_args;
}

void cc_delegate::info(info_str& dest) const
{
	strings::copy(dest, "function call");
};

void cc_delegate::execute(pcstr str)
{
	m_func(str);
};

//--string
cc_string::cc_string(pcstr name, pstr value, u32 size)
:super(name), m_value(value), m_size(size)
{
	m_need_args		= true;
	m_serializable	= true;
}

void cc_string::execute(pcstr args)
{
	strings::copy_n(m_value, m_size, args, m_size);
}

void cc_string::status(status_str& dest) const
{
	strings::copy(dest, m_value);
}

void cc_string::syntax(syntax_str& dest) const
{
	sprintf	(dest, "max size is %d", m_size);
}

void cc_string::info(info_str& dest) const
{
	sprintf	(dest, "string value.");
}

//--bool
cc_bool::cc_bool(pcstr name, bool& value)
:super(name), m_value(value)
{
	m_need_args		= true;
	m_serializable	= true;
}

void cc_bool::execute(pcstr args)
{
	bool v;
	if(strings::equal(args, "on") ||strings::equal(args, "true") ||strings::equal(args, "1"))
		v	= true;
	else
	if(strings::equal(args, "off") ||strings::equal(args, "false") ||strings::equal(args, "0"))
		v	= false;
	else{
		on_invalid_syntax(args);
		return;
	}
	m_value = v;
}

void cc_bool::status(status_str& dest) const
{
	sprintf(dest, "%s", m_value ? "on" : "off");
}

void cc_bool::syntax(syntax_str& dest) const
{
	sprintf	(dest, "on/off, true/false, 1/0");
}

void cc_bool::info(info_str& dest) const
{
	sprintf	(dest, "boolean value.");
}

//--float
cc_float::cc_float(pcstr name, float& value, float const min, float const max)
:super(name, value, min, max)
{}

void cc_float::execute(pcstr args)
{
	float v;
	if(1!=XRAY_SSCANF(args, "%f", &v) || v<m_min || v>m_max)
	{
		on_invalid_syntax(args);
		v = m_min;
		return;
	}
	m_value = v;
}

static void trim_float_str(console_command::status_str& dest)
{
	pstr p = dest+xray::strings::length(dest)-1;
	while(*p=='0' && *(p-1)=='0')
	{
		*p = 0;
		--p;
	}
}

void cc_float::status(status_str& dest) const
{
	sprintf		(dest, "%3.5f", m_value);
	trim_float_str	(dest);
}

void cc_float::info(info_str& dest) const
{
	sprintf	(dest, "floating point value.]");
}

void cc_float::syntax(syntax_str& dest) const
{
	sprintf	(dest, "range [%3.3f,%3.3f]", m_min, m_max);
}
//--u32
cc_u32::cc_u32(pcstr name, u32& value, u32 const min, u32 const max)
:super(name, value, min, max)
{}

void cc_u32::execute(pcstr args)
{
	u32 v;
	if(1!=XRAY_SSCANF(args, "%d", &v) || v<m_min || v>m_max)
	{
		on_invalid_syntax(args);
		v = m_min;
		return;
	}
	m_value = v;
}

void cc_u32::status(status_str& dest) const
{
	sprintf	(dest, "%d", m_value);
}

void cc_u32::info(info_str& dest) const
{
	sprintf	(dest, "unsigned integer value.");
}

void cc_u32::syntax(syntax_str& dest) const
{
	sprintf	(dest, "range [%d,%d]", m_min, m_max);
}
//--float2
cc_float2::cc_float2(pcstr name, float2& value, float2 const min, float2 const max)
:super(name, value, min, max)
{}

void cc_float2::execute(pcstr args)
{
	float2 v;
	if( 2!=XRAY_SSCANF(args, "%f,%f", &v.x, &v.y) || 
		(v.x<m_min.x || v.y<m_min.y || v.x>m_max.x || v.y>m_max.y) )
	{
		on_invalid_syntax(args);
		v = m_min;
		return;
	}
	m_value = v;
}

void cc_float2::status(status_str& dest) const
{
	sprintf	(dest, "%3.5f,%3.5f", m_value.x, m_value.y);
}

void cc_float2::info(info_str& dest) const
{
	sprintf	(dest, "float2 value.");
}

void cc_float2::syntax(syntax_str& dest) const
{
	sprintf	(dest, "range [%3.3f,%3.3f]-[%3.3f,%3.3f]", m_min.x, m_min.y, m_max.x, m_max.y);
}

//--float3
cc_float3::cc_float3(pcstr name, float3& value, float3 const min, float3 const max)
:super(name, value, min, max)
{}

void cc_float3::execute(pcstr args)
{
	float3 v;
	if( 3!=XRAY_SSCANF(args, "%f,%f,%f", &v.x, &v.y, &v.z) || 
		(v.x<m_min.x || v.y<m_min.y || v.z<m_min.z || v.x>m_max.x || v.y>m_max.y || v.y>m_max.z) )
	{
		on_invalid_syntax(args);
		v = m_min;
		return;
	}
	m_value = v;
}

void cc_float3::status(status_str& dest) const
{
	sprintf	(dest, "%3.5f,%3.5f,%3.5f", m_value.x, m_value.y, m_value.z);
}

void cc_float3::info(info_str& dest) const
{
	sprintf	(dest, "float3 value.");
}

void cc_float3::syntax(syntax_str& dest) const
{
	sprintf	(dest, "range [%3.3f,%3.3f,%3.3f]-[%3.3f,%3.3f,%3.3f]", m_min.x, m_min.y, m_min.z, m_max.x, m_max.y, m_max.z );
}

class cc_help :public console_command
{
public:
						cc_help					(pcstr name):console_command(name){}
	virtual void		execute					(pcstr args);
	virtual void		info					(info_str& dest) const 
	{
		xray::strings::copy(dest, "[command] - displays help information on that command.");
	}
};

void cc_help::execute(pcstr args)
{
	xray::console_commands::show_help(args);
}

static cc_help	s_help_cmd("help");

static float s_float_test = 3.0f;
static cc_float s_test_float("test_float", s_float_test, -10.0f, 300.0f);

static u32 s_u32_test = 18;
static cc_u32 s_test_u32("test_u32", s_u32_test, 100, 300);

static float2 s_float2_test(-13.0f, 20.0f);
static cc_float2 s_test_float2("test_float2", s_float2_test, float2(-100,-50), float2(800, 40));

static string64 s_string_test={"sss"};
static cc_string s_test_string("test_string", s_string_test, sizeof(s_string_test));

static bool s_bool_test = true;
static cc_bool s_test_bool("test_bool", s_bool_test);

static cc_delegate	cgf_save_cc( "cfg_save", cc_delegate::Delegate(&xray::console_commands::save), false );
