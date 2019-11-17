////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_base.h"
#include "tool_base.h"
#include "level_editor.h"
#include "command_set_object_transform.h"

namespace xray {
namespace editor {

static void set_rotation_impl	(float3 p, float4x4& transform)
{
	float3 position			= transform.c.xyz();
	float3 rotation			= transform.get_angles_xyz_INCORRECT();
	float4x4 scale_tranform = transform;
	scale_tranform.c.xyz()	= float3(0,0,0);
	scale_tranform			= scale_tranform*invert4x3(create_rotation_INCORRECT(rotation));
	transform				= (scale_tranform * create_rotation_INCORRECT(p));
	transform.c.xyz()		= position;
}

static void set_scale_impl(float3 p, float4x4& transform)
{
	float3 position			= transform.c.xyz();
	float3 rotation			= transform.get_angles_xyz_INCORRECT();
	transform				= create_scale(p)*create_rotation_INCORRECT(rotation);
	transform.c.xyz()		= position;
}

void object_base::set_transform( float4x4 const& transform )
{
	float3 position_prev		= object_position;
	float3 rotation_prev		= object_rotation;
	float3 scale_prev			= object_scale;

	*m_transform				= transform;
	
	if(m_collision.initialized())
		m_collision.set_matrix	(m_transform);

	if(!position_prev.similar(object_position))
		on_property_changed			("position");

	if(!rotation_prev.similar(object_rotation))
		on_property_changed			("rotation");

	if(!scale_prev.similar(object_scale))
		on_property_changed			("scale");
}

void object_base::set_position(float3 p)
{
	float4x4 transform		= *m_transform;
	transform.c.xyz()		= p;
	set_transform			(transform);
}

void object_base::set_rotation(float3 p)
{
	float4x4 transform		= *m_transform;
	set_rotation_impl		(p, transform);
	set_transform			(transform);
}

float3 object_base::get_scale()
{
	float3 rotation			= get_rotation();
	float4x4 scale_tranform = *m_transform;
	scale_tranform.c.xyz()	= float3(0,0,0);
	scale_tranform			= scale_tranform*invert4x3(create_rotation_INCORRECT(rotation));

	return float3(scale_tranform.e00, scale_tranform.e11, scale_tranform.e22);
}

void object_base::set_scale(float3 p)
{
	ASSERT( !math::is_zero(p.x) && !math::is_zero(p.y) && !math::is_zero(p.z));

	float4x4 transform		= *m_transform;
	set_scale_impl			(p, transform);
	set_transform			(transform);
}

void object_base::set_position_revertible(float3 p)
{
	float4x4 transform		= *m_transform;
	transform.c.xyz()		= p;
	
	if(!is_slave_attribute("position"))
		owner()->get_level_editor()->get_command_engine()->run( gcnew command_set_object_transform( owner()->get_level_editor(), this->id(), transform ) );
	else
		set_transform		(transform);
}

void object_base::set_rotation_revertible(float3 p)
{
	float4x4 transform		= *m_transform;
	set_rotation_impl		(p, transform);

	if(!is_slave_attribute("rotation"))
		owner()->get_level_editor()->get_command_engine()->run( gcnew command_set_object_transform( owner()->get_level_editor(), this->id(), transform ) );
	else
		set_transform		(transform);
}

void object_base::set_scale_revertible(float3 p)
{
	ASSERT( !math::is_zero(p.x) && !math::is_zero(p.y) && !math::is_zero(p.z));

	float4x4 transform		= *m_transform;
	set_scale_impl			(p, transform);

	if(!is_slave_attribute("scale"))
		owner()->get_level_editor()->get_command_engine()->run( gcnew command_set_object_transform( owner()->get_level_editor(), this->id(), transform ) );
	else
		set_transform		(transform);
}

float4x4 place_object( object_base^ o, float3 const& position )
{
	XRAY_UNREFERENCED_PARAMETER	( o );
	float4x4 result_matrix		= float4x4().identity();
	result_matrix.c.xyz().set	(position.x, position.y, position.z);
	return						result_matrix;
}

} // namespace editor
} // namespace xray
