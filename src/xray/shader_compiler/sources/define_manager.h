////////////////////////////////////////////////////////////////////////////
//	Created		: 27.05.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef DEFINE_MANAGER_H_INCLUDED
#define DEFINE_MANAGER_H_INCLUDED

namespace xray {
namespace shader_compiler {

struct defination;


struct define_set
{
	defination* define;
	u32 real_index;
	bool usable;

	//for iteration
	u32 index_iter;
	u32 max_iter;
}; // struct define_set

struct conflict_pair
{
	defination* define[2];
};

struct define_value;

struct defination : private boost::noncopyable
{
	defination(const std::string& define_name);

	define_value*		add_value		( const std::string& value		);
	define_value*		has_value		( const std::string& search_key	) const;
	const std::string&  get_name		(								) const { return m_name; }

	u32					get_num_values	(								) const { return m_values.size(); }
	define_value*		get_value		( u32 index						);
	u32 get_name_hash					() const { return m_name_hash; }

private:
	std::vector<define_value*>  m_values;
	std::string					m_name;
	u32							m_name_hash;
}; // struct defination

/////////////////////////////////////////////////////////////////////////////
struct define_value : private boost::noncopyable
{
	define_value(const std::string& value_name);
	const std::string&  get_value		(			 ) const { return m_value;		}
	void				set_undefined	( bool undef )		 { m_undefined = undef; }
	bool				is_undefined	(			 )		 { return m_undefined;	}
	u32					get_value_hash	() const			 { return m_value_hash; }
private:
	std::string m_value;
	bool		m_undefined;
	u32			m_value_hash;
}; // struct define_value

/////////////////////////////////////////////////////////////////////////////
class define_manager :
	private boost::noncopyable 
{
public:
	define_manager					( char const* alt_path_to_defines);
	~define_manager					( );

	bool					 next_defines_set		( );
	std::vector<define_set>& get_defines_set		( ) { return m_defines_set;		 }
	u32						 get_define_set_index	( ) const { return m_define_set_index; }
	void					 print_set				( bool show_define_name );

	bool get_valid						( ) const { return m_valid; }
	
	static u32 hash_string(const std::string& str);

	static std::vector<std::string>	string_hash;
	void sort_sets();

private:
	const u32 get_num_total_defines	( ) const { return m_defines.size(); }

	void on_load					( resources::queries_result & in_result );
	

	defination* add_define			( const std::string& define_id );
	defination* get_define			( u32 index ) const;
	defination* has_define			( const std::string& define_id ) const;

	bool is_conflicted				( const std::vector<define_set>& m_defines_set );

	bool				increase_defines_set	( bool& changed_only_unused, bool& found_in_history );

	bool skip_set();
	bool is_found_in_history();
	u32 calc_num_usable();
private:
	u32										m_num_max_usable_defines;
	u32										m_num_usable_defines;
	bool									m_sorted;
	bool									m_next_set;
	bool									m_valid;
	std::vector<define_set>					m_defines_set;
	u32										m_num_combinations;
	std::vector<conflict_pair>				m_conflicts;
	std::vector<struct defination*>			m_defines;
	
	std::vector< std::vector<define_set> >	m_defines_set_history;
	
	u32										m_define_set_index;
	std::vector<define_set>					m_prev_set;
	bool									m_skip_used;
	bool									m_first_pass;

}; // define_manager

} // namespace shader_compiler
} // namespace xray

#endif // #ifndef DEFINE_MANAGER_H_INCLUDED