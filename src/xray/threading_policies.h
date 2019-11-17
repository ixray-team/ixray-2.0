////////////////////////////////////////////////////////////////////////////
//	Created		: 18.03.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_THREADING_POLICIES_H_INCLUDED
#define XRAY_THREADING_POLICIES_H_INCLUDED

#include <boost/integer_traits.hpp>
#include <boost/type_traits/remove_cv.hpp>

namespace xray {
namespace threading {

class XRAY_CORE_API single_threading_policy 
{
public:
	struct mutex_raii : detail::noncopyable
	{
		mutex_raii(single_threading_policy const&) {}
		~mutex_raii() {}
	};

public:
	template <typename T>
	static	inline	u32	intrusive_ptr_increment	( T& object )
	{
		return	increment(object.m_reference_count);
	}

	template <typename T>
	static	inline	u32	intrusive_ptr_decrement	( T& object )
	{
		return	decrement(object.m_reference_count);
	}

	template <class IntType>
	static	inline	IntType			increment	( IntType& value )
	{
		ASSERT	( value != boost::integer_traits<typename boost::remove_cv<IntType>::type >::const_max );
		return	++value;
	}

	template <class IntType>
	static	inline	IntType			decrement	( IntType& value )
	{
		ASSERT	( value != boost::integer_traits<typename boost::remove_cv<IntType>::type >::const_min );
		return	--value;
	}

	template <class IntType>
	static	inline	IntType			compare_exchange	( IntType& dest, IntType src, IntType comperand )
	{
		IntType out_result	=	dest;
		if ( dest == comperand )
			dest			=	src;

		return					out_result;
	}

	static	inline	pvoid			compare_exchange_pointer	( pvoid& dest, pvoid src, pvoid comperand )
	{
		pvoid out_result	=	dest;
		if ( dest == comperand )
			dest			=	src;

		return					out_result;
	}

					bool			try_lock	() { return true; }
					void			lock		() { ; }
					void			unlock		() { ; }

}; // class single_threading_policy

class XRAY_CORE_API multi_threading_policy_base 
{
public:
	template <typename T>
	static	inline	u32	intrusive_ptr_increment	( T& object )
	{
		return	increment(object.m_reference_count);
	}

	template <typename T>
	static	inline	u32	intrusive_ptr_decrement	( T& object )
	{
		return	decrement(object.m_reference_count);
	}

	template <class IntType>
	static	inline	IntType			increment	( IntType& value )
	{
		ASSERT	( value != boost::integer_traits< typename boost::remove_cv<IntType>::type >::const_max );
		return	interlocked_increment( value );
	}

	template <class IntType>
	static	inline	IntType			decrement	( IntType& value )
	{
		ASSERT	( value != boost::integer_traits< typename boost::remove_cv<IntType>::type >::const_min );
		return	interlocked_decrement( value);
	}

	template <class IntType>
	static	inline	IntType			compare_exchange	( IntType volatile& dest, IntType src, IntType comperand )
	{
		return	interlocked_compare_exchange	( value, src, comperand );
	}

	static	inline	pvoid			compare_exchange_pointer	( atomic_ptr_type& dest, pvoid src, pvoid comperand )
	{
		return	interlocked_compare_exchange_pointer	( dest, src, comperand );
	}

}; // class multi_threading_policy_base

class XRAY_CORE_API multi_threading_mutex_policy : public multi_threading_policy_base
{
public:
	struct mutex_raii : detail::noncopyable
	{
		mutex_raii(multi_threading_mutex_policy & policy) : policy_(policy) { policy.lock(); }
		~mutex_raii() { policy_.unlock(); }
	private:
		multi_threading_mutex_policy &	policy_;
	};

public:
					bool			try_lock	() { return m_mutex.try_lock(); }
					void			lock		() { m_mutex.lock();	}
					void			unlock		() { m_mutex.unlock();	}

private:
	mutex			m_mutex;

}; // class multi_threading_mutex_policy


class XRAY_CORE_API multi_threading_interlocked_policy : public multi_threading_policy_base
{
public:
	struct mutex_raii : detail::noncopyable
	{
		mutex_raii(multi_threading_interlocked_policy& policy) : policy_(policy) { policy.lock(); }
		~mutex_raii() { policy_.unlock(); }
	private:
		multi_threading_interlocked_policy&	policy_;
	};

public:
									multi_threading_interlocked_policy	() : m_lock(0), m_thread_id(0) {}

					bool			try_lock	() 
					{ 
						if ( interlocked_compare_exchange(m_thread_id, threading::current_thread_id(), 0) == 0 )
						{
							ASSERT				(!m_lock);
							m_lock			=	1;
							return				true;
						}

						return					false;
					}

					void			lock		() 
					{ 
						if ( m_thread_id == (s32)threading::current_thread_id() )
						{
							++m_lock;
							return;
						}

						while ( interlocked_compare_exchange(m_thread_id, threading::current_thread_id(), 0) != 0 ) { ; } 
						ASSERT					(!m_lock);
						m_lock				=	1;
					}

					void			unlock		() 
					{ 
						ASSERT					(m_thread_id == (s32)threading::current_thread_id());
						ASSERT					(m_lock);
						--m_lock;
						if ( !m_lock )
							interlocked_exchange	(m_thread_id , 0); 
					}

private:
	u32					m_lock;
	atomic32_type		m_thread_id;

}; // class multi_threading_mutex_policy

} // namespace threading
} // namespace xray

#endif // #ifndef XRAY_THREADING_POLICY_H_INCLUDED