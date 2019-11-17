////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CONFIGS_LUA_CONFIG_VALUE_H_INCLUDED
#define XRAY_CONFIGS_LUA_CONFIG_VALUE_H_INCLUDED

namespace luabind {
	namespace adl {
		class object;
	} // namespace adl
	
	using adl::object;

	namespace detail {
		template<class AccessPolicy>
		class basic_iterator;

		struct basic_access;
	} // namespace detail

	typedef detail::basic_iterator<detail::basic_access> iterator;
} // namespace luabind

namespace xray {

namespace strings {
	class stream;
} // namespace strings

namespace configs {

enum enum_types;
class lua_config_value;

class XRAY_CORE_API	lua_config_iterator {
public:
	IMPLICIT			lua_config_iterator		( ::luabind::iterator const& iterator );
						lua_config_iterator		( lua_config_iterator const& other );
						~lua_config_iterator	( );
	lua_config_iterator& operator =				( lua_config_iterator const& other );
	lua_config_value	key						( ) const;
	lua_config_value	operator *				( ) const;
	bool				operator ==				( lua_config_iterator const& other ) const;
	bool				operator !=				( lua_config_iterator const& other ) const;
	lua_config_iterator& operator ++			( );
	lua_config_iterator operator ++				( int );

private:
	void				fix_up					( );

private:
	enum {
		luabind_handle_sizeof			= ((sizeof(pvoid) + sizeof(int) - 1)/sizeof(pvoid) + 1)*sizeof(pvoid),
		luabind_object_iterator_sizeof	= 2*luabind_handle_sizeof + sizeof(pvoid),
	};
	/*XRAY_DEFAULT_ALIGN*/ char	m_iterator_fake[ luabind_object_iterator_sizeof ];// sizeof(luabind::object::iterator);
	::luabind::iterator*	m_iterator;
}; // class lua_config_iterator

class XRAY_CORE_API lua_config_value {
public:
	IMPLICIT			lua_config_value		( ::luabind::object const& object);
						lua_config_value		( ::luabind::object const& object, ::luabind::object const& table_object, pcstr field_id );
						~lua_config_value		( );

	void				save					( strings::stream& stream, pstr indent, u32 indent_size, u32 indent_level ) const;
	void				save					( pcstr file_path );
	void				clear					( );

						lua_config_value		( lua_config_value const& other );
	lua_config_value&	operator =				( lua_config_value const& other );
	lua_config_value&	assign_lua_value		( lua_config_value const& other );
	void				swap					( lua_config_value& other );

	bool				value_exists			( pcstr field_id ) const;

public:
	typedef lua_config_iterator					iterator;
	typedef iterator							const_iterator;

	iterator			begin					( );
	iterator			end						( );

	const_iterator		begin					( ) const;
	const_iterator		end						( ) const;

	bool				empty					( ) const;
	u32					size					( ) const;

	lua_config_value const operator[ ]			( u32 index ) const;
	lua_config_value const operator[ ]			( int index ) const;
	lua_config_value const operator[ ]			( pcstr field_id ) const;

	lua_config_value	operator[ ]				( u32 index );
	lua_config_value	operator[ ]				( int index );
	lua_config_value	operator[ ]				( pcstr field_id );

private:
	lua_config_value	operator_brackets_impl	( u32 index ) const;
	lua_config_value	operator_brackets_impl	( int index ) const;
	lua_config_value	operator_brackets_impl	( pcstr field_id ) const;

public:
						operator bool			( ) const;
						operator s8				( ) const;
						operator u8				( ) const;
						operator s16			( ) const;
						operator u16			( ) const;
						operator s32			( ) const;
						operator u32			( ) const;
						operator float			( ) const;
						operator pcstr			( ) const;
						operator math::float2	( ) const;
						operator math::float3	( ) const;
						operator math::float4	( ) const;

public:
	lua_config_value	operator =				( pcstr value );
	lua_config_value	operator =				( bool value );
	lua_config_value	operator =				( int value );
	lua_config_value	operator =				( u32 value );
	lua_config_value	operator =				( float value );
	lua_config_value	operator =				( math::float2 const& value );
	lua_config_value	operator =				( math::float3 const& value );
	lua_config_value	operator =				( math::float4 const& value );
	lua_config_value	create_table			( );

public:
	void				erase					( pcstr field_id );
	void				remove_from_parent		( );
	void				rename					( pcstr new_field_id );

	void				add_super_table			( lua_config_value const& super_table );
	void				add_super_table			( pcstr file_table, pcstr super_table );

	void				remove_super_table		( lua_config_value const& super_table );
	void				remove_super_table		( pcstr file_table, pcstr super_table );

public:
	enum_types			get_type				( ) const;
	luabind::object const& get_object			( ) const;
	pcstr				get_field_id			( ) const;

private:
	void				initialize				( ::luabind::object const& object, ::luabind::object const& table_object );
	void				finalize				( );
	void				fix_up					( ) const;	
	void				fix_up_impl				( ) const;	

	template < typename T >
	inline lua_config_value	assign				( T const& value );

private:
	enum {
		luabind_object_sizeof	= ((sizeof(pvoid) + sizeof(int) - 1)/sizeof(pvoid) + 1)*sizeof(pvoid),
	};
	/*XRAY_DEFAULT_ALIGN*/ char	m_object_fake[ luabind_object_sizeof ];			// sizeof(luabind::object);
	::luabind::object*		m_object;
	/*XRAY_DEFAULT_ALIGN*/ char	m_table_object_fake[ luabind_object_sizeof ];	// sizeof(luabind::object);
	::luabind::object*		m_table_object;
	mutable pstr			m_field_id;
	mutable bool			m_fixed_up;
}; // class lua_config_value

} // namespace configs
} // namespace xray

	inline	void		swap					( xray::configs::lua_config_value& left, xray::configs::lua_config_value& right ) { left.swap(right); }

#endif // #ifndef XRAY_CONFIGS_LUA_CONFIG_VALUE_H_INCLUDED