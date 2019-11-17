////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.11.2008
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "xray/buffer_string.h"
#include <stdarg.h>

namespace xray {

//-----------------------------------------------------------------------------------
// operations
//-----------------------------------------------------------------------------------

buffer_string const&   buffer_string::operator += (const value_type* s)
{
	if ( !s )
	{
		return					*this;
	}

	ASSERT(m_end + strings::length(s) < m_max_end);

	while ( *s && m_end < m_max_end )
	{
		*m_end = *s;
		++m_end;
		++s;
	}

	*m_end = NULL;
	return						*this;
}

void   buffer_string::assignf (pcstr const format, ...)
{
	clear						();
	va_list						argptr;
	va_start 					(argptr, format);
	appendf_va_list				(format, argptr);
	va_end	 					(argptr);
}

void   buffer_string::appendf (pcstr const format, ...)
{
	va_list						argptr;
	va_start 					(argptr, format);
	appendf_va_list				(format, argptr);
	va_end	 					(argptr);
}

void   buffer_string::appendf_va_list (pcstr const format, va_list argptr)
{
	char	 					temp_buffer[1024*4 - 8];
	int const char_count	=	vsnprintf(temp_buffer, sizeof(temp_buffer), format, argptr);
	ASSERT	 					(char_count != -1);
	append						((char*)temp_buffer, (char*)temp_buffer + char_count);
}

void   buffer_string::substr (size_type const pos, size_type count, buffer_string * out_dest) const
{
	ASSERT				(out_dest);
	out_dest->clear		();

	if ( count == npos )
		count		=	length() - pos;
	else
	{
		size_type	src_max_count	=	length() > pos ? length() - pos : 0;
		if ( count > src_max_count )
			count	=	src_max_count;

		size_type	dest_max_count	=	out_dest->max_length();
		if ( count > dest_max_count )
			count	=	dest_max_count;
	}

	out_dest->append(m_begin + pos, m_begin + pos + count);
}

void   buffer_string::make_uppercase ()
{
	if ( length() )
		strupr			(m_begin, length() + 1);
}

void   buffer_string::make_lowercase ()
{
	if ( length() )
		strlwr			(m_begin, length() + 1);
}

} // namespace xray