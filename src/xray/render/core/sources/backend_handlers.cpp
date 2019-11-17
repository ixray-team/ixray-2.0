////////////////////////////////////////////////////////////////////////////
//	Created		: 15.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/backend_handlers.h>
#include <xray/render/core/device.h>
#include <xray/render/core/backend.h>
//#include <xray/memory/memory_guard.h>
 

namespace xray {
namespace render_dx10 {

template <enum_shader_type shader_type>
void constants_handler<shader_type>::assign( res_constant_table const * table)
{
	if( m_current && table)
		utils::calc_lists_diff_range( m_current->m_const_buffers, table->m_const_buffers, m_diff_range_start, m_diff_range_end);
	else
	{
		m_diff_range_start = 0;
		m_diff_range_end	= (table==NULL) ? 0 : table->m_const_buffers.size();
	}

	m_current = table;

#ifdef DEBUG
	m_unset_constants.clear();
#endif //DEBUG

	if( m_current)
	{
		// Update hosts
		res_constant_table::c_table::const_iterator it = m_current->m_table.begin();
		res_constant_table::c_table::const_iterator end = m_current->m_table.end();

		res_constant_table::c_buffers const & buffers = m_current->m_const_buffers;

		u32 update_marker = backend::ref().constant_update_counter();
		for( ; it != end; ++it)
		{
			constant_host * host = it->host;
			ASSERT( host);
			host->shader_slots[shader_type]		= it->slot;

			// Mark constant hos as updated (currently in use) if its source the same with the 
			// current constant being set. In other words if the sources are different then 
			// The constant is bing marked old to deny constant manual setup form backend.
			host->update_markers[shader_type]	= (host->source.ptr == it->source.ptr) ? update_marker : update_marker-1;

			if( host->source.ptr != it->source.ptr)
				LOG_INFO( "The global constant binding for \"%s\"was ignored.", host->name.c_str());

			// Gather constant bound data.
			u32 buff_ind = it->slot.buffer_index;
			if( buff_ind != slot_dest_buffer_null && it->source.ptr != NULL) 
			{
				ASSERT( *((u32*)it->source.ptr) != xray::memory::debug::freed_memory, "The bound object was deleted !");

				buffers[buff_ind]->set( *it);
			}
#ifdef DEBUG
			else if( it->source.ptr == NULL)
			{
				m_unset_constants.push_back( host);
			}
#endif // DEBUG
		}

		//gather_data();
	}
}

template <enum_shader_type shader_type>
void constants_handler <shader_type>::gather_data()
{
	res_constant_table::c_table::const_iterator	it	= m_current->m_table.begin();
	res_constant_table::c_table::const_iterator	end	= m_current->m_table.end();

	res_constant_table::c_buffers const & buffers = m_current->m_const_buffers;
	for ( ; it!=end; ++it)
	{
		u32 buff_ind = it->slot.buffer_index;
		if( buff_ind != slot_dest_buffer_null && it->source.ptr != NULL) 
		{
			ASSERT( *((u32*)it->source.ptr) != xray::memory::debug::freed_memory, "The bound object was deleted !");

			buffers[buff_ind]->set( *it);
		}
	}
}

template <enum_shader_type shader_type>
void  constants_handler<shader_type>::fill_changes_buffer( ID3DConstantBuffer** buffer)
{
	u32 const start	= m_diff_range_start;
	u32 const end	= m_diff_range_end;
	u32 const list_size = (m_current.c_ptr() == NULL) ? 0 : m_current->m_const_buffers.size();

	for( u32 i = start; i < end; ++i)
	{
		if( i < list_size)
			buffer[i]		= m_current->m_const_buffers[i]->get_hw_buffer();
		else
			// Check if we really need to set NULL into unused slots.
			buffer[i]		= 0;
	}
}

template <enum_shader_type shader_type>
void constants_handler<shader_type>::update_buffers()
{
	if( m_current)
	{
		res_constant_table::c_buffers::const_iterator	it = m_current->m_const_buffers.begin();
		res_constant_table::c_buffers::const_iterator	end = m_current->m_const_buffers.end();

		for( ; it != end; ++it)
			(*it)->update();
	}
}

template <>
void constants_handler<enum_shader_type_vertex>::apply	()
{
	check_for_unset_constants();

	u32 const start	= m_diff_range_start;
	u32 const end	= m_diff_range_end;

	if( end-start == 0) 
		return;

	ID3DConstantBuffer*	tmp_buffer[cb_buffer_max_count];
	fill_changes_buffer( tmp_buffer);

	device::ref().d3d_context()->VSSetConstantBuffers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

#ifdef DEBUG
template <enum_shader_type shader_type>
void	constants_handler<shader_type>::check_for_unset_constants()
{
	for( u32 i = 0; i< m_unset_constants.size(); ++i)
		LOG_ERROR( "The constant \"%s\" was not specified for %s!", m_unset_constants[i]->name.c_str(), shader_type_traits<shader_type>::name());
}
#endif // DEBUG

template <>
void constants_handler<enum_shader_type_geometry>::apply	()
{
	check_for_unset_constants();

	u32 const start	= m_diff_range_start;
	u32 const end	= m_diff_range_end;

	if( end-start == 0) 
		return;

	ID3DConstantBuffer*	tmp_buffer[cb_buffer_max_count];
	fill_changes_buffer( tmp_buffer);

	device::ref().d3d_context()->GSSetConstantBuffers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template <>
void constants_handler<enum_shader_type_pixel>::apply	()
{
	check_for_unset_constants();

	u32 const start	= m_diff_range_start;
	u32 const end	= m_diff_range_end;

	if( end-start == 0) 
		return;

	ID3DConstantBuffer*	tmp_buffer[cb_buffer_max_count];
	fill_changes_buffer( tmp_buffer);

	device::ref().d3d_context()->PSSetConstantBuffers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template class constants_handler<enum_shader_type_vertex>;
template class constants_handler<enum_shader_type_geometry>;
template class constants_handler<enum_shader_type_pixel>;


//////////////////////////////////////////////////////////////////////////
//textures 
template <enum_shader_type shader_type>
void textures_handler<shader_type>::assign( res_texture_list const * list)
{
	u32 start, end;
	if( m_current && list)
		utils::calc_lists_diff_range( *m_current, *list, start, end);
	else
	{
		start	= 0;
		const u32 curr_list_size =	(m_current.c_ptr() == NULL) ? 0 : m_current->size();
		const u32 new_list_size =	(list == NULL) ? D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT : list->size();

		// This is done to be sure that after several assignments the total range will include all that assignment changes.
		end	= math::max( curr_list_size, new_list_size);
	}
	m_diff_range_start	= start;	
	m_diff_range_end	= math::max( m_diff_range_end, end);

	m_current = list;
}

template <enum_shader_type shader_type>
void  textures_handler<shader_type>::fill_changes_buffer( ID3DShaderResourceView** buffer)
{
	const u32 start		= m_diff_range_start;
	const u32 end		= m_diff_range_end;
	const u32 list_size = (m_current.c_ptr() == NULL) ? 0 : m_current->size();

	for( u32 i = start; i < end; ++i)
	{
		if( i < list_size && (*m_current)[i])
			buffer[i]		= (*m_current)[i]->get_view ();
		else
			// Check if we really need to set NULL into unused slots.
			buffer[i]		= 0;
	}
}

#ifdef DEBUG
template <enum_shader_type shader_type>
void textures_handler<shader_type>::check_for_unset_textures( texture_slots const & slots)
{
	for( int i = 0; i < D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT ; ++i)
		if( slots[i].name != "" && ( (*m_current)[i].c_ptr() == NULL || (*m_current)[i]->get_view() == NULL))		
			LOG_ERROR( "Texture \"%s\" was not specified for %s. ", slots[i].name, shader_type_traits<shader_type>::name());
}
#endif // DEBUG

template <>
void textures_handler<enum_shader_type_vertex>::apply	()
{
	ID3DShaderResourceView*	tmp_buffer[D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->VSSetShaderResources( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template <>
void textures_handler<enum_shader_type_geometry>::apply	()
{
	ID3DShaderResourceView*	tmp_buffer[D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->GSSetShaderResources( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template <>
void textures_handler<enum_shader_type_pixel>::apply	()
{
	ID3DShaderResourceView*	tmp_buffer[D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->PSSetShaderResources( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template class textures_handler<enum_shader_type_vertex>;
template class textures_handler<enum_shader_type_geometry>;
template class textures_handler<enum_shader_type_pixel>;


//////////////////////////////////////////////////////////////////////////
/// samplers
template <enum_shader_type shader_type>
void samplers_handler<shader_type>::assign( res_sampler_list const * list)
{
	if( m_current && list)
		utils::calc_lists_diff_range( *m_current, *list, m_diff_range_start, m_diff_range_end);
	else
	{
		m_diff_range_start	= 0;
		m_diff_range_end	= (list == NULL) ? D3D_COMMONSHADER_SAMPLER_SLOT_COUNT : list->size();
	}

	m_current = list;
}

template <enum_shader_type shader_type>
void  samplers_handler<shader_type>::fill_changes_buffer( ID3DSamplerState ** buffer)
{
	const u32 start		= m_diff_range_start;
	const u32 end		= m_diff_range_end;
	const u32 list_size = (m_current.c_ptr() == NULL) ? 0 : m_current->size();

	for( u32 i = start; i < end; ++i)
	{
		if( i < list_size)
			buffer[i]		= (*m_current)[i];
		else
			// Check if we really need to set NULL into unused slots.
			buffer[i]		= 0;
	}
}

#ifdef DEBUG
template <enum_shader_type shader_type>
void samplers_handler<shader_type>::check_for_unset_samplers( sampler_slots const & slots)
{
	for( int i = 0; i < D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT ; ++i)
		if( slots[i].name != "" && !(*m_current)[i] )		
			LOG_ERROR( "Sampler \"%s\" was not for %s.", slots[i].name, shader_type_traits<shader_type>::name());
}
#endif // DEBUG

template <>
void samplers_handler<enum_shader_type_vertex>::apply	()
{
	ID3DSamplerState*	tmp_buffer[D3D_COMMONSHADER_SAMPLER_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->VSSetSamplers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template <>
void samplers_handler<enum_shader_type_geometry>::apply	()
{
	ID3DSamplerState*	tmp_buffer[D3D_COMMONSHADER_SAMPLER_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->GSSetSamplers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template <>
void samplers_handler<enum_shader_type_pixel>::apply	()
{
	ID3DSamplerState*	tmp_buffer[D3D_COMMONSHADER_SAMPLER_SLOT_COUNT];

	fill_changes_buffer( tmp_buffer);
	u32 start	= m_diff_range_start;
	u32 end		= m_diff_range_end;

	if( end-start > 0) 
		device::ref().d3d_context()->PSSetSamplers( start, end-start, &tmp_buffer[start]);

	m_diff_range_start = m_diff_range_end = 0;
}

template class samplers_handler<enum_shader_type_vertex>;
template class samplers_handler<enum_shader_type_geometry>;
template class samplers_handler<enum_shader_type_pixel>;


} // namespace render 
} // namespace xray 

