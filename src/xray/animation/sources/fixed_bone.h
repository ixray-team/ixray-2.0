////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef FIXED_BONE_H_INCLUDED
#define FIXED_BONE_H_INCLUDED

#include "poly_curve.h"

namespace xray {

namespace configs
{
	class lua_config_value;
} // namespace configs

namespace animation {

class skeleton;

struct fixed_bone {

	fixed_bone(): m_anim_skeleton_idx( u32(-1) ), m_fixed_vertex( float3(0,0,0 ) ){}
		
	u32		m_anim_skeleton_idx;	
	float3	m_fixed_vertex;
	
	void read( configs::lua_config_value const& config, const animation::skeleton &s );

}; // struct fixed_bone





struct animation_foot_steps
{
	
	static const u32 max_intervals = 11;
	fixed_vector< float, max_intervals > intervals;

}; // struct animation_foot_steps

struct foot_steps_bones
{
	
	fixed_vector< u32, animation_foot_steps::max_intervals - 1 > bones;

}; // struct foot_steps_bones

struct foot_steps
{
	
	foot_steps_bones				m_bones;
	vector< animation_foot_steps >	m_steps;
	
	inline	u32		intervals_size	( )const { return m_steps[ 0 ].intervals.size(); }
			void	get_intervals	( buffer_vector< float > &intervals,  const buffer_vector< float > &weights )const;

			u32		bone			( float time, const buffer_vector< float > &weights, float& interval_start )const;
			u32		bone			( float time, const buffer_vector< float > &weights )const;

}; // struct foot_steps


class fixed_bones:
	private boost::noncopyable
{

public:
	fixed_bones( const animation::skeleton &s ): m_skeleton( s ){}

	void				read			( configs::lua_config_value const& config  );
	void				read_steps		( foot_steps &steps, float play_time,  configs::lua_config_value const& config )const;

	inline const fixed_bone	&bone		( u32 idx_fixed )const	{ return m_bones[ idx_fixed ]; }

private:
	u32					skeleton_bone_idx( pcstr name )const;
	u32					fixed_bone_idx	( pcstr name )const;

private:
	const animation::skeleton		&m_skeleton;
	vector< fixed_bone >			m_bones;

}; //struct fixed_bones




} // namespace animation
} // namespace xray

#endif // #ifndef FIXED_BONE_H_INCLUDED