////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_VS_HW_H_INCLUDED
#define RES_VS_HW_H_INCLUDED

#include "res_xs_hw.h"
#include "xs_data.h"


namespace xray {
namespace render_dx10 {


typedef res_xs_hw<vs_data>	res_vs_hw;

typedef xray::intrusive_ptr<res_vs_hw, res_base, threading::single_threading_policy>	ref_vs_hw;
typedef xray::intrusive_ptr<res_vs_hw const, res_base const, threading::single_threading_policy>	ref_vs_hw_const;

// class res_vs_hw : 
// 	public res_named,
// 	public res_id,
// 	public boost::noncopyable
// {
// public:
// 	res_vs_hw();
// 	~res_vs_hw();
// 
// 	HRESULT create_hw_shader(ID3DBlob* shader_code);
// 	
// 	void apply() { backend::ref().set_vs(m_vs);}
// 	void merge_ctable_in( res_constant_table& dest) { dest.merge(m_constants);}
// 
// private:
// 	ID3DVertexShader*		m_vs;			//vs;
// 	res_constant_table		m_constants;	//constants;
// 	ref_signature			m_signature;	//signature;
// };
// typedef xray::intrusive_ptr<res_vs_hw, res_base, threading::single_threading_policy>	ref_vs_hw;
// 

} // namespace render
} // namespace xray

#endif // #ifndef RES_VS_HW_H_INCLUDED