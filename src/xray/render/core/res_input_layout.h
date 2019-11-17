////////////////////////////////////////////////////////////////////////////
//	Created		: 15.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_INPUT_LAYOUT_H_INCLUDED
#define RES_INPUT_LAYOUT_H_INCLUDED

#include "res_vs_hw.h"

namespace xray {
namespace render_dx10 {

class res_declaration;

class res_input_layout :
	public res_flagged,
	public boost::noncopyable
{
	friend class resource_manager;
public:
	res_input_layout	( res_declaration const * decl, res_signature const * shader);
	~res_input_layout	();
	void _free			() const;

	bool equal( res_declaration const * decl, res_signature const * signature) const
	{
		return ( m_declaration == decl && m_signature == signature);
	}
	
	ID3DInputLayout * hw_layout() { return m_hw_input_layout;}

private:

	ID3DInputLayout *		m_hw_input_layout;
	res_declaration const *	m_declaration;
	ref_signature_const		m_signature;

}; // class res_input_layout

typedef xray::intrusive_ptr<res_input_layout, res_base, threading::single_threading_policy>			ref_input_layout;
typedef xray::intrusive_ptr<res_input_layout const, res_base const, threading::single_threading_policy>	ref_input_layout_const;


} // namespace render
} // namespace xray

#endif // #ifndef RES_INPUT_LAYOUT_H_INCLUDED