////////////////////////////////////////////////////////////////////////////
//	Created 	: 24.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

namespace stalker2 {
class game;

class game_object :	public resources::unmanaged_resource,
					private boost::noncopyable 
{
public:
					game_object				( game& game );
	virtual			~game_object			( );
	virtual void	load					( xray::configs::lua_config_value const& t );
	virtual void	decrease_quality		( )		{ NOT_IMPLEMENTED();}
	virtual void	make_quality_optimal	( )		{ NOT_IMPLEMENTED();}

private:

protected:
	game&			m_game;
	float4x4		m_transform;
}; // class world

typedef	intrusive_ptr<
			game_object,
			resources::unmanaged_intrusive_base,
			threading::single_threading_policy
		> game_object_ptr;

} // namespace stalker2

#endif // #ifndef OBJECT_H_INCLUDED