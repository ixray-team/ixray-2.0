////////////////////////////////////////////////////////////////////////////
//	Created		: 05.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef REGRESSION_TREE_H_INCLUDED
#define REGRESSION_TREE_H_INCLUDED

#include "iaction.h"

namespace xray {

namespace configs{
	class lua_config_value;
} // namespace configs

namespace rtp {

template< class space_param >
class training_save_load_data;

template< class space_param >
class node_save_load_data
{
public:
	virtual training_save_load_data<space_param>* get_training_save_load_data() { return 0; }

public:
	virtual const training_save_load_data< space_param >* get_training_save_load_data()const { return 0; }
	virtual	~node_save_load_data	() { }
}; // class node_save_load_data


template< class space_param >
class node
{
public:
typedef	space_param space_param_type ;

public:
			node		();
	virtual	~node		();
	float	reward		(  const space_param_type &params  ) const ;

protected:
			bool	is_leaf		( ) const;

public://protected?
	virtual	bool	build		( ){ ASSERT(false); return false; }
	virtual	bool	recalculate ( ){ ASSERT(false); return false; }

public:
	virtual	void	save		( xray::configs::lua_config_value cfg, const node_save_load_data< space_param_type >& data )const;
	virtual	void	load		( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data  );

private:
	virtual	node	*create		( u32 level );
	virtual	u32		level		( ) { return u32(-1); };
protected:
		u32										m_split_dimension;
		float									m_split_value;
		node									*left, *right ;//, *parent;
		float									m_reward_value;
}; // class node


template< class node >
class regression_tree 
{
public:
	typedef				node node_type;

protected:
	typedef typename	node_type::space_param_type	space_param_type; 

public:
		regression_tree( ):m_root( 0 ){};

protected:
		regression_tree	( u32 min_samples ): m_root( 0, min_samples ){};
		//regression_tree	(  const typename node::training_sample_type **begin,  const typename node::training_sample_type **end, const space_param_type &min, const space_param_type &max, u32 min_samples ): m_root( 0,  begin, end, min, max, min_samples ){};
virtual	~regression_tree(){}

public:
float	reward			(  const space_param_type &params  ) const 
	{
		return m_root.reward( params );
	}

public:
virtual	void	save		( xray::configs::lua_config_value cfg, const node_save_load_data< space_param_type >& data  )const;
virtual	void	load		( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data );

protected:
	node_type			m_root;

}; // class regression_tree

} // namespace rtp
} // namespace xray

#include "regression_tree_inline.h"

#endif // #ifndef REGRESSION_TREE_H_INCLUDED