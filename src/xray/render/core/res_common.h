////////////////////////////////////////////////////////////////////////////
//	Created		: 07.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef RES_COMMON_DX11_H_INCLUDED
#define RES_COMMON_DX11_H_INCLUDED


#include <xray/render/core/sources/com_utils.h>

namespace xray {
namespace render_dx10 {


typedef render::vector<D3D_SHADER_MACRO> shader_defines_list;
void make_defines(shader_defines_list& defines);
void define_macro(shader_defines_list& defines, const char* param);
void define_macro(shader_defines_list& defines, const char* param, const char* value);

// resource itself, the base class for all derived resources
class res_base
{
public:
	res_base() : m_reference_count(0) {}
	res_base( const res_base&) : m_reference_count(0) {}

	template < typename T >
	static inline void destroy(T* object);// {DELETE(object);}

public:
	mutable u32	m_reference_count;	//dwReference;
};

class res_flagged : public res_base
{
public:
	res_flagged(): m_flags(0) {}


	bool registered() const {return m_flags & res_flag_registered;}

protected:
	void mark_registered() {m_flags |= res_flag_registered;}

private:
	enum { res_flag_registered = 1<<0};

public:
	u32	m_flags;	//dwFlags;
};

class res_named	: public res_flagged
{
public:
	res_named()	: m_name(0) {}
	~res_named() {}

	inline const char* set_name(const char * name)	{m_name = name; return m_name.c_str();}
	inline const char* get_name() const {return m_name.c_str();}

public:
	shared_string	m_name;	//cName;
};

class res_id
{
	//friend class CResourceManager;

public:
	res_id(): m_id((u32)-1) {}

	u32  get_id() {return m_id;}
	void set_id(u32 id) {m_id = id;}

private:
	u32	m_id;
};

#include "res_common_inline.h"

} // namespace render 
} // namespace xray 


#endif // #ifndef RES_COMMON_DX11_H_INCLUDED