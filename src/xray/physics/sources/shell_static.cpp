////////////////////////////////////////////////////////////////////////////
//	Created 	: 16.02.2008
//	Author		: Konstantin Slipchenko
//	Description : shell static
////////////////////////////////////////////////////////////////////////////


#include "pch.h"
#include "shell_static.h"
#include "shell_element.h"
#include "static_element.h"

shell_static::shell_static		( static_element	&e )
:m_element(&e)
{

}

shell_static::~shell_static		( )
{
	DELETE( m_element );
}

void	shell_static::collide( island& i, space_object& o )						
{
	o.collide_detail( i, *this );
}

bool	shell_static::collide_detail( island& i, space_object& o )
{
	return o.collide( i, *m_element );
	
}

bool	shell_static::collide( island& i, collide_element_interface& e )
{
	return m_element->collide( i, e );
}

//void	shell_static::island_connect( island	&i )
//{
//	UNREACHABLE_CODE();
//}

void	shell_static::render( render::debug::renderer& renderer )const
{
	XRAY_UNREFERENCED_PARAMETER	( renderer );
	//m_element->render			( renderer );
}

void	shell_static::aabb( float3 &center, float3 &radius )const	
{
	math::aabb box;
	ASSERT( m_element );
	center = m_element->aabb(box).center( radius );

}

bool	shell_static::aabb_test			( math::aabb const& aabb ) const
{
	ASSERT( m_element );
	return m_element->aabb_test( aabb );
}

void	shell_static::space_check_validity( )const
{
	ASSERT(m_element, "static physics shell check space validity fail! ");
	m_element->check_collision_validity();
}



