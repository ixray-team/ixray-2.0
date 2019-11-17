////////////////////////////////////////////////////////////////////////////
//	Created		: 24.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

namespace xray {
namespace rtp {

static const u32 samples_number_pruning_threshhold	= 60000;
static const u32 num_prune_steps					= 2;
static const u32 max_itteration						= 15;//15;
static const u32 num_trajectories					= 10;



extern bool b_learn_trees_frozen;
extern bool	b_learn_prune;

template< class action >
training_value_function< action >::training_value_function( const action_base<action_type>& ab ): 
	super( ab ),
	m_steps_tries( 0 ),
	m_steps_samples( 0 ),
	m_prune_steps( 0 )
{
	
}


template< class action >
training_value_function< action >::~training_value_function( )
{
	super::clear();
}

template< class action >
void training_value_function< action >::init_trees( )
{
	clear();
	clear_training_sets();
	const u32 sz = m_action_base.size();
	m_regression_trees.resize( sz, 0 );
	m_training_sets.resize( sz,  training_set<training_sample_type>() );
	for( u32 i = 0; i<sz; ++i )
		m_regression_trees[i] = NEW(regression_tree_type)( m_training_sets[i] );
	for( u32 i = 0; i < num_trajectories * sz ; ++i )
		m_trajectories.push_back(trajectory<action_type>(i));
	
	m_steps_tries	= 0;
	m_steps_samples	= 0;
	m_prune_steps	= 0;

}

template< class action >
bool	training_value_function<action>::samples_step( )
{
	m_add_samples_mutex.lock();
	
	bool samples = m_steps_samples <= m_steps_tries; 



	if( samples )
	{
		if( !b_learn_trees_frozen )
			generate_samples();
		++m_steps_samples;
	}

	m_add_samples_mutex.unlock();

	return samples;
}

template< class action >
void training_value_function<action>::build_trees( )
{
	
	
	m_add_samples_mutex.lock();
	
	bool build = m_steps_tries < m_steps_samples ; 

	typename vector< training_set< training_sample_type > >::iterator i = m_training_sets.begin(),
												   e = m_training_sets.end();
	
	for( ; i != e ; ++i )
		( *i ).synchronize();
		
	m_add_samples_mutex.unlock();


	if( !build )
		return;

	const u32 sz = m_regression_trees.size();

	m_building_regression_trees.resize( sz, 0 );

	for( u32 i = 0; i<sz; ++i )
	{
		//DELETE( m_regression_trees[i] );
		m_training_sets[i].randomize();
		m_building_regression_trees[i] = NEW(regression_tree_type)( m_training_sets[i] );
		m_building_regression_trees[i]->build();
	}

	m_add_samples_mutex.lock();

	for( u32 i = 0; i<sz; ++i )
	{
		DELETE( m_regression_trees[i] );
		
		m_regression_trees[i]=m_building_regression_trees[i];
		m_building_regression_trees[i] = 0;
	}

	++m_steps_tries;

	m_add_samples_mutex.unlock();


}

template< class action >
void training_value_function< action >::recalculate_trees( )
{
	const u32 sz = m_regression_trees.size();
	
	for( u32 i = 0; i<sz; ++i )
		m_regression_trees[i]->recalculate();

}





template< class action >
float	training_value_function< action >::generate_samples( )
{
	
	
	clear_trajectories			( );

	const u32 size				= m_action_base.size();

	buffer_vector<u32> rnd_actions( ALLOCA( sizeof(32) * size ), size, size );
	
	//const u32 min_add_samples					= size * num_trajectories * max_itteration / 40;
	u32	samples_added = 0;
	//float max_residual = 0;
	//while( samples_added < min_add_samples )
		for( u32 k = 0; k < num_trajectories; ++k )
		{
			

			set_random_buffer( rnd_actions );

			for( u32 j = 0; j < size; ++j )
			{
				space_param_type start; start.set_random();
				space_param_type pcurrent = start;

				u32 action_id = rnd_actions[ j ];
				const action* current = m_action_base.get( action_id );
				u32			  current_blend_sumple = 0; //rnd.random(max_samples);

				//if(	current->is_taget_action()	&& ! current->taget( pcurrent ) )
					//continue;

				trajectory<action_type> &current_trajectory = m_trajectories[ k * size + j ];
						
				u32 i = 0;
				for( ; i < max_itteration; ++i )
				{
					float r = 0, v = 0; 
					space_param_type pnext ;//=  pcurrent;
					u32 blend(u32(-1));
					const action *next = pi( *current, current_blend_sumple, pcurrent, blend, pnext, r, v );// r = R(a,a') + discount * V(s') ; v = V( s') ; s' = (next,pnext)
					ASSERT(math::valid(r));
					ASSERT(math::valid(v));
					//float value = nu( *current,  pcurrent );
					float value = r + state_reward( *current,  pcurrent );
					ASSERT( math::valid( value ) );

					
							
					
					if( !in_area( pcurrent )  )
					{
						if( current->remove_out_of_area_trajectory() )
							current_trajectory.clear();
						//i--;
						break;
					}
					current_trajectory.add( regression_tree_type::training_sample_type( pcurrent, value ), current );	

					current_trajectory.set_is_in_taget( true );

					if(  current->taget( pcurrent ) )
						break;

					///
					//if( current->is_taget_action() )
					//{
					//	current_trajectory.clear();
					//	break;
					//}
					///

					current_trajectory.set_is_in_taget( false );

					//m_trajectories[ k * size + j ].clear();
					//i--;
					//if(	current->is_taget_action() )
						//break;

					//LOG_DEBUG("pcurrent %f, %f", pcurrent[0], pcurrent[1]  );
					//LOG_DEBUG("next %f, %f", pnext[0], pnext[1]  );

					current				 = next;
					current_blend_sumple = blend;
					pcurrent			 = pnext;

				}

				//if( i!= max_itteration )
				samples_added += current_trajectory.size();
				current_trajectory.add_to_sets( m_training_sets );
			}

	}
	return 0;
}

template< class action >
void	training_value_function< action >::render		( xray::render::debug::renderer& renderer ) const
{
	typename rtp::vector< trajectory<action_type> >::const_iterator i = m_trajectories.begin(), e = m_trajectories.end();
	for ( ; i != e ; ++i )
		(*i).render( renderer );
}

template< class action >
void	training_value_function< action >::clear_trajectories  ( )
{
	typename rtp::vector< trajectory<action_type> >::iterator i = m_trajectories.begin(), e = m_trajectories.end();
	for ( ; i != e ; ++i )
		(*i).clear();
}

template< class action >
void training_value_function< action >::learn( )
{
	init_trees();
	for( u32 k = 0; k < 1000000; ++k )
		learn_step( );
}

template< class action >
void training_value_function<action>::learn_init( )
{
	if(m_regression_trees.empty())
		init_trees();
}

template< class action >
u32	training_value_function< action >::samples_total( )
{
	const u32 size = m_action_base.size();
	u32 total_samples = 0;
	for( u32 j= 0; j < size; ++j )
		total_samples += m_training_sets[j].samples().size();
	return total_samples;
}


template< class action >
void training_value_function< action >::clear_training_sets()
{
	const u32 size = m_training_sets.size();
	for(u32 j= 0; j < size; ++j )
		m_training_sets[j].samples().clear();
}


template< class action >
float	training_value_function< action >::update_sets( )
{
	const u32 size = m_action_base.size();
	float sum_sq_residual = 0;
	u32 total_samples = 0;
	for(u32 j= 0; j < size; ++j )
	{
		const action& a = *m_action_base.get( j );
		rtp::vector< typename regression_tree_type::training_sample_type >& set = m_training_sets[j].samples();
		typename rtp::vector< typename regression_tree_type::training_sample_type >::iterator i = set.begin(), e  = set.end();
		for(;i!=e;++i)
		{
			(*i).second = nu( a, 0, (*i).first ); //. add previous blending state to samples?
			float diff = (*i).second - value( a, (*i).first ) ;
			sum_sq_residual += diff * diff;
			++total_samples;
		}
	}
	if( total_samples == 0 )
		return math::float_max;
	return math::sqrt( sum_sq_residual/total_samples );
}

template<class action>
void training_value_function< action >::randomize_sets( )
{
	const u32 num_actions = m_action_base.size	( );
	for( u32 id_action = 0; id_action <  num_actions; ++id_action )
	{
		m_training_sets[ id_action ].randomize();
	}
}

template<class action>
void training_value_function< action >::prune_build_tree( u32 min_samples, u32 id_action )
{
	//const u32 num_actions = m_action_base.size	( );	
	//for( u32 id_action = 0; id_action <  num_actions; ++id_action )
	{
		training_set<training_sample_type>		&set	= m_training_sets[id_action];
		regression_tree_type*					&tree	= m_regression_trees[id_action]; 
		const u32 sz = set.samples().size();
		ASSERT( sz >= 3 );
		const u32 sz_div_3  = u32( sz / 3 );
		const u32 sz_2d3	= sz - sz_div_3;

		//rtp::vector<const training_sample_type*> smaller_set;
		//static const u32 size = set.samples().size();

		buffer_vector<const training_sample_type*>	smaller_set(  ALLOCA( sizeof( training_sample_type ) * sz_2d3 ) , sz_2d3 );
		//set.randomize();
		for( u32 i = 0; i < sz_2d3; ++i )
			smaller_set.push_back( &set.samples()[i] );
		DELETE(tree);
		tree = NEW(regression_tree_type)( min_samples, smaller_set );
		tree->build();
	}
}

template< class action >
void training_value_function< action >::prune( )
{
	//b_prune =true;
	randomize_sets( );

	const u32 num_actions = m_action_base.size( );
	for( u32 id_action = 0; id_action <  num_actions; ++id_action )
	{
		float min_residual = math::infinity;
		u32 min_samples_best = u32(-1);
		for( u32 min_samples = 1; min_samples <= regression_tree_type::node_type::min_samples_max; ++min_samples )
		{
			prune_build_tree( min_samples, id_action );
			float res = prune_residual();//prune_residual(id_action);//prune_residual();
			if( res <= min_residual )
			{
				min_residual		= res;
				min_samples_best	= min_samples;
			}
		}
		training_set<training_sample_type>		&set	= m_training_sets[id_action];
		set.min_in_leaf_samples( ) = min_samples_best;
		LOG_DEBUG( "rtp::prune action :%d - min_samples: %d", id_action, min_samples_best );
	}
	clear_training_sets();
	//b_prune =false;
}

template<class action>	
float	training_value_function< action >::prune_sum_residual( u32 id_action, u32 &number )
{
		training_set<training_sample_type>		&set	= m_training_sets[id_action];
//		regression_tree_type*					&tree	= m_regression_trees[id_action]; 

		const u32 sz = set.samples().size();

		ASSERT( sz >= 3 );

		const u32 sz_div_3  = u32( sz / 3 );
		const u32 sz_2d3	= sz - sz_div_3;
		u32 from = sz_2d3;
		u32 to = sz;
		ASSERT( from < to );
		ASSERT( to <= sz );

		const action *a = m_action_base.get(id_action);
		float sum = 0;
		for( u32 i = from; i < to  ; ++i )
		{
			float diff = nu( *a, 0, set.samples()[i].first ) - value( *a, set.samples()[i].first ); //.add previous blending state to samples?
			sum += diff*diff;
		}
		number += ( to - from );
		return sum;
}

template< class action >
float training_value_function< action >::prune_residual( u32 id_action )
{
	
	u32 number = 0;
	float sum = prune_sum_residual( id_action, number );
	return math::sqrt( sum/number );
}

template< class action >
float training_value_function< action >::prune_residual( )
{
	const u32 num_actions = m_action_base.size( );
	float sum = 0;
	u32 number = 0;
	for( u32 id_action = 0; id_action <  num_actions; ++id_action )
	{
		u32 num = 0;
		sum +=prune_sum_residual( id_action, num );
		number+=num;
		//training_set<training_sample_type>		&set	= m_training_sets[id_action];
		//regression_tree_type*					&tree	= m_regression_trees[id_action]; 

		//const u32 sz = set.samples().size();

		//ASSERT( sz >= 3 );

		//const u32 sz_div_3  = u32( sz / 3 );
		//const u32 sz_2d3	= sz - sz_div_3;
		//u32 from = sz_2d3;
		//u32 to = sz;
		//ASSERT( from < to );
		//ASSERT( to <= sz );

		//const action *a = m_action_base.get(id_action);
		//
		//for( u32 i = from; i < to  ; ++i )
		//{
		//	float diff = nu( *a, set.samples()[i].first ) - value( *a, set.samples()[i].first );
		//	sum += diff*diff;
		//}
		//number += ( to - from );
	}

	return math::sqrt( sum/number );
}


template< class action >
float			training_value_function< action >::learn_step( )
{

		//if( !b_learn_trees_frozen )
			//generate_samples(  );
		//clear();

		if( !b_learn_trees_frozen )
			build_trees();
		else
			recalculate_trees();

		float residual = update_sets();

		//recalculate_trees(); //?

		if( samples_total() > samples_number_pruning_threshhold * ( m_prune_steps*2 + 1 )  && m_prune_steps < num_prune_steps && b_learn_prune )
		{
			prune();
			++m_prune_steps;
			//b_learn_prune = false;
		}

		return residual;
}

template< class action >
void training_value_function< action >::save( xray::configs::lua_config_value cfg, bool training )const
{
	
	
	if( m_regression_trees.empty() )
		training = false;

	cfg["training"] = training;
	if(!training)
	{
		super::save( cfg );
		return;
	}
	
	cfg["prune_steps"] = m_prune_steps;

	save_training_sets( cfg );

	const u32 sz = m_action_base.size();
	for( u32 i = 0; i<sz; ++i )
	{
		training_save_load_data< space_param_type > data( m_training_sets[i].samples(), *m_regression_trees[i] );
		m_regression_trees[i]->save( cfg["tries"][i], data );
	}
}

template< class action >
void	training_value_function< action >::load( const xray::configs::lua_config_value &cfg )
{
	const bool training = cfg["training"];
	if(!training)
	{
		super::load( cfg );
		return;
	}
	

	clear();
	init_trees();
	

	m_prune_steps = cfg["prune_steps"];

	load_training_sets( cfg );

	const u32 sz = m_action_base.size();
	for( u32 i = 0; i < sz; ++i )
	{
		training_save_load_data< space_param_type > data( m_training_sets[i].samples(), *m_regression_trees[i] );

		DELETE( m_regression_trees[i] );

		m_regression_trees[i] = NEW(regression_tree_type)( m_training_sets[i] ); 

		m_regression_trees[i]->load( cfg["tries"][i], data );

	}
}

template< class action >
void training_value_function< action >::save_training_sets( xray::configs::lua_config_value cfg )const
{
	xray::configs::lua_config_value cfg_sets = cfg["training_sets"];
	cfg_sets["number"] = m_training_sets.size();
	typename rtp::vector< training_set< training_sample_type> >::const_iterator  i, b  = m_training_sets.begin(),
		e = m_training_sets.end();
	for(i = b;i!=e;++i)
		(*i).save(cfg_sets["sets"][u32(i-b)]);
}

template< class action >
void training_value_function< action >::load_training_sets	( const xray::configs::lua_config_value &cfg )
{
	const xray::configs::lua_config_value cfg_sets = cfg["training_sets"];
	const u32 sz =  (u32)cfg_sets["number"];
	m_training_sets.resize( sz );

	typename rtp::vector< training_set< training_sample_type> >::iterator i, b = m_training_sets.begin(), 
		e = m_training_sets.end();
	for( i = b; i!=e; ++i )
		(*i).load( cfg_sets["sets"][u32(i-b)] );
}

} // namespace rtp
} // namespace xray