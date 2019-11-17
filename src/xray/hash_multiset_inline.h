#ifndef HASH_MULTISET_INLINE_INCLUDED
#define HASH_MULTISET_INLINE_INCLUDED

#define TEMPLATE_SIGNATURE  template <class Value, class SizePolicy, class HashFunc, class EqualKey, class ThreadingPolicy>
#define HASH_MULTISET		xray::hash_multiset<Value, SizePolicy, HashFunc, EqualKey, ThreadingPolicy>

namespace xray
{

//-----------------------------------------------------------------------------------
// class hash_multiset_intrusive_base
//-----------------------------------------------------------------------------------

template <class Derived>
class hash_multiset_intrusive_base
{
private:
	Derived*	m_next_hash_node;

public:
	hash_multiset_intrusive_base() : m_next_hash_node(NULL)		{}

	void		set_next_hash_node (Derived* node)	{ m_next_hash_node = node; }
	Derived*	get_next_hash_node () const			{ return m_next_hash_node; }
};

//-----------------------------------------------------------------------------------
// class hash_multiset::iterator
// class hash_multiset::reverse_iterator
//-----------------------------------------------------------------------------------

TEMPLATE_SIGNATURE
class HASH_MULTISET::iterator
{
public:																					
	iterator				operator ++ ();
	iterator				operator ++ (int);
	Value*					operator *	();
	const Value*			operator *	() const;
	bool					operator == (iterator const & it) const;
	bool					operator != (iterator const & it) const { return !(*this == it); }

private:
	hash_multiset*			m_container;
	Value*					m_value;
	int						m_index;

	template <class , class , class , class , class >
	friend class hash_multiset;
};

TEMPLATE_SIGNATURE								
class HASH_MULTISET::reverse_iterator : protected
	  HASH_MULTISET::iterator
{
public:																					
	reverse_iterator		operator ++ ();
	reverse_iterator		operator ++ (int);

	bool					operator == (const reverse_iterator& it) { return (iterator&)*this == it; }
	bool					operator != (const reverse_iterator& it) { return !(*this == it); }
};

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator   
		 HASH_MULTISET::iterator::operator ++ ()
{
	ASSERT(m_container);
	if ( !m_value ) 
	{
		return *this; // end
	}

	m_value			= m_value->get_next_hash_node();

	while ( !m_value )
	{
		++m_index;

		if ( m_index >= SizePolicy::fixed_size )
		{
			break;
		}

		m_value		= m_container->m_buffer[m_index];
	}

	return *this;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator   
		 HASH_MULTISET::iterator::operator ++ (int)
{
	iterator t(*this);
	++*this;
	return t;
}

TEMPLATE_SIGNATURE 
Value*   HASH_MULTISET::iterator::operator * ()
{
	return	 m_value;
}

TEMPLATE_SIGNATURE
const Value*   HASH_MULTISET::iterator::operator * () const
{
	return	 m_value;
}

TEMPLATE_SIGNATURE
bool   HASH_MULTISET::iterator::operator == (iterator const & it) const
{
	ASSERT				(m_container	==	it.m_container);
	return				 m_value		==	it.m_value;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::reverse_iterator
		 HASH_MULTISET::reverse_iterator::operator ++ ()
{
	ASSERT				(m_container);

	if ( !m_value )
	{
		return			*this; // end
	}

	Value* res	=	m_container->m_buffer[m_index];

	while ( res	&& res->get_next_hash_node() != m_value )
	{
		res			=	res->get_next_hash_node();
	}

	if ( !res )
	{
		while ( m_index >= 0 )
		{
			--m_index;

			if ( (res = m_container->m_buffer[m_index]) != NULL )
			{
				while ( res->get_next_hash_node() )
				{
					res = res->get_next_hash_node();
				}
				
				break;
			}
		}
	}
	
	m_value			=	res;
	return				*this;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::reverse_iterator   
		 HASH_MULTISET::reverse_iterator::operator ++ (int)
{
	reverse_iterator t(*this);
	++*this;
	return t;
}

//-----------------------------------------------------------------------------------
// class hash_multiset
//-----------------------------------------------------------------------------------

TEMPLATE_SIGNATURE
HASH_MULTISET::hash_multiset ()
{
	memory::zero	(m_buffer, sizeof(m_buffer));
}

TEMPLATE_SIGNATURE
HASH_MULTISET::~hash_multiset ()
{
	clear();
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator	
		 HASH_MULTISET::begin ()
{
	iterator it;
	it.m_container		=	this;
	it.m_index			=	0;
	it.m_value			=	NULL;

	for ( int		i	=	0; 
					i	<	SizePolicy::fixed_size; 
				  ++i )
	{
		if	( m_buffer[i] )
		{
			it.m_index	=	i;
			it.m_value	=	m_buffer[i];
			break;
		}
	}

	return it;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator	
		 HASH_MULTISET::end ()
{
	iterator it;
	it.m_container			=	this;
	it.m_index				=	SizePolicy::fixed_size;
	it.m_value				=	NULL;
	return it;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::reverse_iterator	
HASH_MULTISET::rbegin ()
{
	reverse_iterator it;
	it.m_container		=	this;
	it.m_value			=	NULL;

	for ( int i			=	SizePolicy::fixed_size-1; 
			  i			>=	0; 
		    --i )
	{
		if	( Value* value = m_buffer[it.m_index] )
		{
			while ( value->get_next_hash_node() )
			{
				value	=	value->get_next_hash_node();
			}

			it.m_value	=	value;
			break;
		}
	}

	it.m_index = i;

	return it;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::reverse_iterator	
HASH_MULTISET::rend ()
{
	reverse_iterator it;
	it.m_container			=	this;
	it.m_index				=	0;
	it.m_value				=	NULL;
	return it;
}

TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator   
		 HASH_MULTISET::find (Value* const value)
{
	typename ThreadingPolicy::mutex_raii raii(m_threading_policy);

	u32 const index			=	u32( m_hash( *value ) ) % SizePolicy::fixed_size;
	Value * candidate		=	m_buffer[ index ];
	while ( candidate )
	{
		if ( m_equal_key(*candidate, *value) )
		{
			iterator it;
			it.m_container	= this;
			it.m_index		= index;
			it.m_value		= candidate;
			return it;
		}

		candidate			=	candidate->get_next_hash_node();
	}

	return end();
}


TEMPLATE_SIGNATURE 
typename HASH_MULTISET::iterator  
		 HASH_MULTISET::find (const u32 hash)
{
	typename ThreadingPolicy::mutex_raii raii(m_threading_policy);

	u32 const index		=	hash % SizePolicy::fixed_size;
	Value * candidate	=	m_buffer[ index ];
	if ( candidate )
	{
		iterator it;
		it.m_container	= this;
		it.m_index		= index;
		it.m_value		= candidate;
		return it;
	}

	return end();
}

TEMPLATE_SIGNATURE 
void   HASH_MULTISET::insert (Value* const value)
{
	insert(m_hash(*value), value);
}

TEMPLATE_SIGNATURE 
void	HASH_MULTISET::insert (u32 hash, Value* const value)
{
	typename ThreadingPolicy::mutex_raii raii(m_threading_policy);
	u32 const index			=	hash % SizePolicy::fixed_size;
	Value** element			=	&m_buffer[ index ];
	value->set_next_hash_node	( *element );
	*element					=	value;
	++m_size;
}

template <class Value>
struct empty_deleter { inline void operator () (Value*) const {} };

TEMPLATE_SIGNATURE
void   HASH_MULTISET::clear ()
{
	empty_deleter<Value> deleter;
	clear<empty_deleter<Value> >( deleter );
}

TEMPLATE_SIGNATURE
template <class Deleter>
void   HASH_MULTISET::clear ( Deleter const& deleter )
{
	typename ThreadingPolicy::mutex_raii raii(m_threading_policy);

	for (				u32 i	=	0;
							i	<	SizePolicy::fixed_size; 
						  ++i )
	{
		Value * current		=	m_buffer[i];
		while ( current ) {
			Value* next		=	current->get_next_hash_node();
			deleter			( current );
			current			=	next;
		}

		m_buffer[i]			=	NULL;
	}

	m_size = 0;
}

TEMPLATE_SIGNATURE
Value*   HASH_MULTISET::erase (const iterator& it)
{
	ASSERT(it.m_container == this);
	erase(it.m_index, it.m_value);
	return it.m_value;
}

TEMPLATE_SIGNATURE
void   HASH_MULTISET::erase (Value * value)
{
	erase								(m_hash(* value), value);
}

TEMPLATE_SIGNATURE
void   HASH_MULTISET::erase (const u32 hash, Value* value)
{
	typename ThreadingPolicy::mutex_raii raii(m_threading_policy);

	ASSERT(value);

	u32			index				=	hash % SizePolicy::fixed_size;
	Value*		cur_node			=	m_buffer[index];

	if ( cur_node == value )
	{
		m_buffer[index]				= value->get_next_hash_node();
	}
	else
	{
		while ( cur_node->get_next_hash_node() != value )
		{
			cur_node				=	cur_node->get_next_hash_node();
			ASSERT(cur_node);
		}

		cur_node->set_next_hash_node	( value->get_next_hash_node() );
	}

	--m_size;

	value->set_next_hash_node			(NULL);
}

} // namespace xray

#undef HASH_MULTISET
#undef TEMPLATE_SIGNATURE

#endif // HASH_MULTISET_INLINE_INCLUDED