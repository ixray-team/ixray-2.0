////////////////////////////////////////////////////////////////////////////
//	Created 	: 10.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_INTRUSIVE_PTR_INLINE_H_INCLUDED
#define XRAY_INTRUSIVE_PTR_INLINE_H_INCLUDED

#define TEMPLATE_SIGNATURE \
	template <\
		typename object_type,\
		typename base_type,\
		typename threading_policy\
	>

#define INTRUSIVE_PTR \
	xray::intrusive_ptr<\
		object_type,\
		base_type,\
		threading_policy\
	>

TEMPLATE_SIGNATURE
inline INTRUSIVE_PTR::intrusive_ptr												( )
{
	m_object		= 0;
}

TEMPLATE_SIGNATURE
inline INTRUSIVE_PTR::intrusive_ptr												( object_type* const object )
{
	m_object		= 0;
	set				( object );
}

TEMPLATE_SIGNATURE
inline INTRUSIVE_PTR::intrusive_ptr												( self_type const& other )
{
	m_object		= 0;
	set				( other );
}

TEMPLATE_SIGNATURE
inline INTRUSIVE_PTR::~intrusive_ptr											( )
{
	dec				( );
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_PTR::dec													( )
{
	if (!m_object)
		return;

	if ( !threading_policy::intrusive_ptr_decrement(static_cast<base_type&>(*m_object)) )
		m_object->base_type::destroy	( m_object );
}

TEMPLATE_SIGNATURE
inline object_type &INTRUSIVE_PTR::operator*									( ) const
{
	ASSERT			( m_object );
	return			( *m_object );
}

TEMPLATE_SIGNATURE
inline object_type *INTRUSIVE_PTR::operator->									( ) const
{
	ASSERT			( m_object );
	return			( m_object );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::operator!	( ) const
{
	return			( !m_object );
}

TEMPLATE_SIGNATURE
inline INTRUSIVE_PTR::operator typename INTRUSIVE_PTR::unspecified_bool_type	( ) const
{
	if ( !m_object )
		return		( 0 );

	return			( &intrusive_ptr::c_ptr );
}

TEMPLATE_SIGNATURE
inline typename INTRUSIVE_PTR::self_type &INTRUSIVE_PTR::operator=				( object_type* const object )
{
	self_type( object ).swap( *this );
	return			( *this );
}

TEMPLATE_SIGNATURE
inline typename INTRUSIVE_PTR::self_type &INTRUSIVE_PTR::operator=				( self_type const& object )
{
	self_type( object ).swap( *this );
	return			( *this );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::operator==											( INTRUSIVE_PTR const& object ) const
{
	return			( c_ptr( ) == object.c_ptr( ) );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::operator!=											( INTRUSIVE_PTR const& object ) const
{
	return			( c_ptr( ) != object.c_ptr( ) );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::operator<											( INTRUSIVE_PTR const& object ) const
{
	return			( c_ptr( ) < object.c_ptr( ) );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::operator>											( INTRUSIVE_PTR const& object ) const
{
	return			( c_ptr( ) > object.c_ptr( ) );
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_PTR::swap													( self_type& object )
{
	std::swap		( m_object, object.m_object );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_PTR::equal												( self_type const& object )	const
{
	return			( m_object == object.m_object );
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_PTR::set													( object_type* const object )
{
	if ( m_object == object )
		return;
	
	dec				( );

	m_object		= object;
	if (!m_object)
		return;

	threading_policy::intrusive_ptr_increment	( static_cast<base_type&>(*m_object) );
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_PTR::set													( self_type const& object )
{
	if (m_object == object.m_object)
		return;

	dec				( );
	m_object		= object.m_object;
	
	if (!m_object)
		return;

	threading_policy::intrusive_ptr_increment	( static_cast<base_type&>(*m_object) );
}

TEMPLATE_SIGNATURE
inline object_type* INTRUSIVE_PTR::c_ptr										( )	const
{
	return			( m_object );
}

TEMPLATE_SIGNATURE
inline void swap																( INTRUSIVE_PTR& left, INTRUSIVE_PTR& right )
{
	left.swap		( right );
}


template < typename dest_type, typename object_type, typename base_type, typename threading_policy >
dest_type xray::static_cast_resource_ptr										(
		xray::intrusive_ptr<
			object_type,
			base_type,
			threading_policy
		> const src_ptr
	)
{
	return			static_cast_checked<typename dest_type::object_type_t*>(src_ptr.c_ptr());
}

#undef INTRUSIVE_PTR
#undef TEMPLATE_SIGNATURE

#endif // #ifndef XRAY_INTRUSIVE_PTR_INLINE_H_INCLUDED