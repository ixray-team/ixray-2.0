////////////////////////////////////////////////////////////////////////////
//	Created		: 07.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef RES_DECLARATION_H_INCLUDED
#define RES_DECLARATION_H_INCLUDED

#include "res_input_layout.h"
#include "res_vs_hw.h"

namespace xray {
namespace render_dx10 {

struct signiture_layout_pair
{ 
	signiture_layout_pair( res_declaration const* decl, res_signature const * shader);

	bool operator < ( res_signature const * other ) const { return &*signature < other;}
	bool operator == ( res_signature const * other ) const { return &*signature == other;}

	ref_input_layout	input_layout;
	ref_signature_const	signature;
};


class res_declaration :
	public res_flagged,
	public boost::noncopyable
{
	friend class resource_manager;
	friend class res_base;

	res_declaration		( D3D_INPUT_ELEMENT_DESC const* decl, u32 count);
	~res_declaration	();
	void _free			() const;
public:

	bool equal( D3D_INPUT_ELEMENT_DESC const* other, u32 count) const
	{
		if (count != dcl_code.size())
			return false;

		return memcmp(&dcl_code[0], other, count*sizeof(D3D_INPUT_ELEMENT_DESC)) == 0;
	}

	inline res_input_layout * get( res_signature const * signature);

// --Porting to DX10_
//	u32 get_stride(u32 stream) {return D3DXGetDeclVertexSize(&*m_dcl_code.begin(), stream);}
// --Porting to DX10_
//	void apply() {backend::ref().set_decl( this);}

	render::vector<signiture_layout_pair>			vs_to_layout;	
	render::vector<D3D_INPUT_ELEMENT_DESC> const	dcl_code;				
};

typedef xray::intrusive_ptr<res_declaration, res_base, threading::single_threading_policy>	ref_declaration;
typedef xray::intrusive_ptr<res_declaration const, res_base const, threading::single_threading_policy>	ref_declaration_const;


//////////////////////////////////////////////////////////////////////////
// inline implementations
res_input_layout * res_declaration::get( res_signature const * signature) 
{ 
	render::vector<signiture_layout_pair>::iterator it = std::lower_bound( vs_to_layout.begin(), vs_to_layout.end(), signature);

	if( it != vs_to_layout.end() &&  *it == signature)
		return &*it->input_layout;

	return &*vs_to_layout.insert( it, signiture_layout_pair( this, signature))->input_layout;
}


} // namespace render 
} // namespace xray 


#endif // #ifndef RES_DECLARATION_H_INCLUDED
