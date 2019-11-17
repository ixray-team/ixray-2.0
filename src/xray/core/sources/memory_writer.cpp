////////////////////////////////////////////////////////////////////////////
//	Created		: 11.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/memory_writer.h>
#include <xray/fs_utils.h>
#include <xray/linkage_helper.h>

DECLARE_LINKAGE_ID(core_memory_writer)

namespace xray {
namespace memory {

i_writer::i_writer( memory::base_allocator* allocator )
:m_allocator	( allocator ),
m_chunk_pos		( allocator )			
{
}

i_writer::~i_writer( )	
{
	ASSERT	(m_chunk_pos.empty(),"Opened chunk not closed.");	
}


void i_writer::open_chunk	(u32 type)
{
	w_u32					(type);
	m_chunk_pos.push_back	(tell());
	w_u32					(0);	// the place for 'size'
}

void i_writer::close_chunk	()
{
	ASSERT			(!m_chunk_pos.empty());

	int pos			= tell();
	seek			(m_chunk_pos.back());
	w_u32			(pos-m_chunk_pos.back()-4);
	seek			(pos);
	m_chunk_pos.pop_back	();
}

u32 i_writer::chunk_size	()					// returns size of currently opened chunk, 0 otherwise
{
	if (m_chunk_pos.empty())	
		return 0;

	return tell() - m_chunk_pos.back()-4;
}

void i_writer::w_chunk(u32 type, void* data, u32 size)
{
	open_chunk	(type);
	w			(data,size);
	close_chunk	();
}


writer::writer(memory::base_allocator* allocator) 
:i_writer( allocator )
{
	data		= 0;
	position	= 0;
	mem_size	= 0;
	file_size	= 0;
	external_data= false;
}

writer::~writer() 
{	free_();	}

void writer::free_()			
{
	file_size=0; 
	position=0; 
	mem_size=0; 
	if(!external_data)
		XRAY_FREE_IMPL( m_allocator, data );
}

void writer::w	(const void* ptr, u32 count)
{
	if (position+count > mem_size) 
	{

		// reallocate
		if (mem_size==0)	mem_size=128;
		while (mem_size <= (position+count)) mem_size*=2;
		if (0==data)		data = (pbyte)	XRAY_MALLOC_IMPL( m_allocator, mem_size, "memory_writer" );
		else				
			data = (pbyte)	XRAY_REALLOC_IMPL	( m_allocator, data, mem_size, "memory_writer" );
	}
	xray::memory::copy		( data+position, mem_size-position, ptr, count );
	position				+=count;

	if (position>file_size) 
		file_size=position;
}

bool writer::save_to	(pcstr fn)
{
	FILE*		f;
	bool success = fs::open_file(&f, fs::open_file_write | fs::open_file_create | fs::open_file_truncate, fn);
	ASSERT_U	( success, "cant open file for writing ", fn);

	fwrite		(pointer(), size(), 1, f);

	fclose		(f);

	return		false;
}

} // namespace memory
} // namespace xray
