////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef VALUE_FUNCTION_H_INCLUDED
#define VALUE_FUNCTION_H_INCLUDED

#include "regression_tree.h"
#include "action.h"
#include <xray/math_randoms_generator.h>

namespace xray {

namespace configs{
	class lua_config_value;
} // namespace configs

namespace rtp {

template< class regression_tree, class action >
class value_function:
	private boost::noncopyable
{
	typedef	regression_tree								regression_tree_type;
	typedef	action										action_type;
	typedef	typename action_type::space_param_type		space_param_type;

public:
		value_function		( const action_base< action_type >& ab );
virtual	~value_function		( ){}

public:
	float			core_pi		( const	action_type &afrom, u32 from_blend_id, const action_type &ato, u32 &blend_id, const space_param_type &from_param , space_param_type &to_param, float &max_v )const;
	float			nu			( const	action_type &afrom, u32 from_blend_id, const space_param_type &from_param ) const;

public:
const action_type	*pi			( const action_type &from, u32 from_blend_id, const space_param_type &from_param, u32 &blend_id,  space_param_type &to_params, float &max_reward, float &v ) const;
const action_type	*pi			( const action_type &from, u32 from_blend_id, const space_param_type &from_param, u32 &blend_id ) const;

public:
		float		value		( const	action_type &ato, const space_param_type &to_param )const;
virtual	void		render		( xray::render::debug::renderer& renderer ) const;

public:
	void			save		( xray::configs::lua_config_value cfg )const;
	void			load		( const xray::configs::lua_config_value &cfg );
	bool			empty		( )const;

protected:
	void			clear		( );

private:
virtual	void		init_trees	( );

protected:
	action_base< action_type >const						&m_action_base;

	rtp::vector< regression_tree_type* >				m_regression_trees;

}; // class value_function

} // namespace rtp
} // namespace xray

#include "value_function_inline.h"

#endif // #ifndef VALUE_FUNCTION_H_INCLUDED