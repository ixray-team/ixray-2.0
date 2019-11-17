////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CONSOLE_COMMAND_H_INCLUDED
#define XRAY_CONSOLE_COMMAND_H_INCLUDED

namespace xray {
namespace console_commands{

class XRAY_CORE_API console_command : private detail::noncopyable
{
public:
	typedef	string512	info_str;
	typedef	string512	status_str;
	typedef	string512	syntax_str;
public:
						console_command			(pcstr name);
	virtual				~console_command		();
	virtual void		execute					(pcstr args)				= 0;
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
			pcstr		name					() const					{return m_name;}
			bool		need_args				() const					{return m_need_args;}
			bool		serializable			() const					{return m_serializable;}
			void		serializable			(bool b)					{m_serializable = b;}

	console_command*	prev					() const					{return m_prev;}
	console_command*	next					() const					{return m_next;}
protected:
			void		on_invalid_syntax		(pcstr args);
			
	console_command*	m_next;
	console_command*	m_prev;
	pcstr				m_name;
	bool				m_need_args;
	bool				m_serializable;
};//class console_command

typedef fastdelegate::FastDelegate< void (pcstr) >		Delegate;
template class XRAY_CORE_API fastdelegate::detail::ClosurePtr< Delegate::GenericMemFn, Delegate::StaticFunctionPtr, Delegate::UnvoidStaticFunctionPtr >;
template class XRAY_CORE_API fastdelegate::FastDelegate< void (pcstr) >;
template class XRAY_CORE_API fastdelegate::FastDelegate1< pcstr, void >;

class XRAY_CORE_API cc_delegate :public console_command
{
public:
	typedef Delegate	Delegate;

public:
						cc_delegate				(pcstr name, Delegate const& func, bool need_args);
	virtual void		execute					(pcstr args);
	virtual void		info					(info_str& dest) const;

protected:
			Delegate	m_func;

private:
	typedef console_command	super;
};// cc_string

class XRAY_CORE_API cc_string :public console_command
{
	typedef console_command		super;
public:
						cc_string				(pcstr name, pstr value, u32 size);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
protected:
	pstr				m_value;
	u32					m_size;
};// cc_string

class XRAY_CORE_API cc_bool :public console_command
{
	typedef console_command		super;
public:
						cc_bool					(pcstr name, bool& value);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
protected:
	bool&				m_value;
};// cc_bool

#pragma warning(push)
#pragma warning(disable:4251)
template<class T>
class XRAY_CORE_API cc_value : public console_command
{
	typedef console_command		super;
public:
	inline			cc_value				(pcstr name, T& value, T const min, T const max);
protected:
			T&		m_value;
			T		m_min;
			T		m_max;
};//cc_value
#pragma warning(pop)

class XRAY_CORE_API cc_float :public cc_value<float>
{
	typedef cc_value<float>		super;
public:
						cc_float				(pcstr name, float& value, float const min, float const max);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
};//cc_float 

class XRAY_CORE_API cc_u32 :public cc_value<u32>
{
	typedef cc_value<u32>		super;
public:
						cc_u32					(pcstr name, u32& value, u32 const min, u32 const max);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
};//cc_u32

class XRAY_CORE_API cc_float2 :public cc_value<math::float2>{
	typedef cc_value<math::float2>	super;
public:
						cc_float2				(pcstr name, math::float2& value, math::float2 const min, math::float2 const max);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
}; // class cc_float2

class XRAY_CORE_API cc_float3 :public cc_value<math::float3>
{
	typedef cc_value<math::float3>	super;
public:
						cc_float3				(pcstr name, math::float3& value, math::float3 const min, math::float3 const max);
	virtual void		execute					(pcstr args);
	virtual void		status					(status_str& dest) const;
	virtual void		info					(info_str& dest) const;
	virtual void		syntax					(syntax_str& dest) const;
}; // class cc_float3

} // namespace console_commands
} // namespace xray

#include <xray/console_command_inline.h>

#endif // #ifndef XRAY_CONSOLE_COMMAND_H_INCLUDED