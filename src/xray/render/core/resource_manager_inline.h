////////////////////////////////////////////////////////////////////////////
//	Created		: 24.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCE_MANAGER_INLINE_H_INCLUDED
#define RESOURCE_MANAGER_INLINE_H_INCLUDED

namespace xray {
namespace render_dx10 {

template< int size >
inline res_geometry* resource_manager::create_geometry( D3D_INPUT_ELEMENT_DESC const (&decl)[size], u32 vertex_stride, res_buffer* vb, res_buffer* ib)
{
	return create_geometry( (D3D_INPUT_ELEMENT_DESC*)&decl, size, vertex_stride, vb, ib);
}

void resource_manager::copy		( res_buffer* dest, res_buffer* source)
{
	device::ref().d3d_context()->CopyResource( dest->hw_buffer(), source->hw_buffer());
}

void resource_manager::copy		( res_buffer* dest, u32 dest_pos, res_buffer* source, u32 src_pos, u32 size)
{
	D3D_BOX box;
	box.left	= src_pos;
	box.top		= 0;
	box.front	= 0;
	box.right	= src_pos + size;
	box.bottom	= 1;
	box.back	= 1;

	device::ref().d3d_context()->CopySubresourceRegion( dest->hw_buffer(), 0, dest_pos, 0, 0, source->hw_buffer(), 0, &box);
}

} // namespace render
} // namespace xray

#endif // #ifndef RESOURCE_MANAGER_INLINE_H_INCLUDED