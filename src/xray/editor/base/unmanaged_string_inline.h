////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef UNMANAGED_STRING_INLINE_H_INCLUDED
#define UNMANAGED_STRING_INLINE_H_INCLUDED

inline unmanaged_string::unmanaged_string	( System::String^ string )
{
	int const buffer_size		= int( (string->Length + 1) * 2 );
	m_string					= ( pstr ) MALLOC( (u32)buffer_size, "unmanaged string" );
	pin_ptr<wchar_t const> wch	= PtrToStringChars( string );
	LPCSTR const default_character	= "_";
	size_t const converted_characters_count =
		WideCharToMultiByte(
			GetACP(),
			0,
			wch,
			buffer_size,
			m_string,
			buffer_size,
			default_character,
			NULL
		);
	R_ASSERT					( converted_characters_count, "[tostring][failed] : WideCharToMultiByte failed" );
}


inline unmanaged_string::~unmanaged_string	( )
{
	FREE					( m_string );
}

inline pstr unmanaged_string::c_str			( ) const
{
	return					( m_string );
}

#endif // #ifndef UNMANAGED_STRING_INLINE_H_INCLUDED