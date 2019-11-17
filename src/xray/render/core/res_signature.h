////////////////////////////////////////////////////////////////////////////
//	Created		: 28.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_SIGNATURE_H_INCLUDED
#define RES_SIGNATURE_H_INCLUDED

#include "res_common.h"

namespace xray {
namespace render_dx10 {

//////////////////////////////////////////////////////////////////////////
// res_signature 
class res_signature :
	public res_flagged,
	public boost::noncopyable
{
	friend class res_base;
	friend class resource_manager;

	res_signature	( ID3DBlob * singniture_blob): m_signature (singniture_blob){ singniture_blob->AddRef();}
	~res_signature	() { safe_release(m_signature); }
	void _free() const;
public:

	ID3DBlob *			hw_signiture() const { return m_signature;}

	bool equal ( ID3DBlob const * other)  { return other == m_signature; }

private:
	ID3DBlob *			m_signature;	//signature;
};
typedef xray::intrusive_ptr<res_signature, res_signature, threading::single_threading_policy>				ref_signature;
typedef xray::intrusive_ptr<res_signature const, res_signature const, threading::single_threading_policy>	ref_signature_const;

} // namespace render
} // namespace xray

#endif // #ifndef RES_SIGNATURE_H_INCLUDED