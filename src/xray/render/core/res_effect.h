////////////////////////////////////////////////////////////////////////////
//	Created		: 07.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef RES_EFFECT_H_INCLUDED
#define RES_EFFECT_H_INCLUDED

#include "res_state.h"
#include "res_xs.h"
#include "res_effect.h"
#include "res_declaration.h"


namespace xray {
namespace render_dx10 {

//////////////////////////////////////////////////////////////////////////
class res_pass : public res_flagged, public boost::noncopyable
{
	friend class effect_manager;
	friend class res_base;

public:
	res_pass( const ref_vs& vs, const ref_gs& gs, const ref_ps& ps, const ref_state& state):
	  m_state(state), 
		  m_vs(vs),
		  m_gs(gs), 
		  m_ps(ps)
	  {}
	  ~res_pass	();
	  void _free	() const;

	 void init_layout( res_declaration const & decl);

	bool equal(const res_pass& other) const;
	bool equal(const res_pass* other) const {return equal(*other);}

	void apply() const;

	// Need review...
	inline u64 get_priority() const;

public:
	ref_state			m_state;			//state;		// Generic state, like Z-Buffering, samplers, etc
	ref_vs				m_vs;				
	ref_gs				m_gs;			
	ref_ps				m_ps;				

	ref_input_layout	m_input_layout;	// May be NULL
};

typedef intrusive_ptr<res_pass, res_base, threading::single_threading_policy>	ref_pass;

//////////////////////////////////////////////////////////////////////////
// shader_technique
class res_shader_technique : public res_flagged/*, public boost::noncopyable*/
{
public:
	struct sflags
	{
		u32	priority		: 2; //iPriority
		u32	strict_b2f		: 1; //bStrictB2F
		u32	has_emissive	: 1; //bEmissive
		u32	has_distort		: 1; //bDistort
		u32	has_wmark		: 1; //bWmark
	};


	friend class effect_manager;
	res_shader_technique	() {}
	~res_shader_technique	();
	void _free();

public:
	void init_layouts( res_declaration const & decl);

	bool equal(const res_shader_technique& other) const;
	bool equal(const res_shader_technique* other) const {return equal(*other);}

	res_pass const* get_pass( u32 i) const { return &*m_passes[i];}

public:
	static const int SHADER_PASSES_MAX = 2;

	sflags										m_flags;
	fixed_vector<ref_pass,SHADER_PASSES_MAX>	m_passes;
};

typedef intrusive_ptr<res_shader_technique, res_base, threading::single_threading_policy>	ref_shader_technique;

//////////////////////////////////////////////////////////////////////////
// effect
class res_effect :
	public res_flagged
{
	friend class effect_manager;
	friend class effect_compiler;
	friend class res_base;

	~res_effect();
	void _free();

public:

	void init_layouts( res_declaration const & decl);

	bool equal(const res_effect& other) const;
	bool equal(const res_effect* other) const {return equal(*other);}
	
	void apply (u32 technique_id = 0, u32 pass_id = 0) {select_technique(technique_id); apply_pass(pass_id);}

	void select_technique(u32 technique_id) {ASSERT(technique_id<m_techniques.size()); m_cur_technique = technique_id;}
	void apply_pass(u32 pass_id);

	res_shader_technique const * get_technique( u32 i) const		{ return &*m_techniques[i];}

private:
	static const int SHADER_TECHNIQUES_MAX = 6;

	u32	m_cur_technique;
	fixed_vector<ref_shader_technique, SHADER_TECHNIQUES_MAX>	m_techniques;
	//R1-0=norm_lod0(det), 1=norm_lod1(normal), 2=L_point, 3=L_spot, 4=L_for_models,	
	//R2-0=deffer, 1=norm_lod1(normal), 2=psm, 3=ssm, 4=dsm
};

typedef intrusive_ptr<res_effect, res_base, threading::single_threading_policy> ref_effect;


//#define		SE_R2_NORMAL_HQ		0	// high quality/detail
//#define		SE_R2_NORMAL_LQ		1	// low quality
//#define		SE_R2_SHADOW		2	// shadow generation
//	E[3] - can use for night vision but need to extend SE_R1. Will need 
//	Extra shader element.
//	E[4] - distortion or self illumination(self emission). 
//	E[4] Can use for lightmap capturing.

//#pragma pack(pop)


u64 res_pass::get_priority() const
{
#ifdef DEBUG
	const int vs_id_bit_width = 7;
#endif // #ifdef DEBUG
	const int ps_id_bit_width = 7;
	const int state_id_bit_width = 7;
//	const int tex_list_id_bit_width = 12;

	u64 qword = 0;
	u64 vs_id = m_vs->get_id();
	ASSERT(vs_id < (1<<vs_id_bit_width));
	qword = vs_id;

	qword <<= ps_id_bit_width;
	u64 ps_id = m_ps->get_id();
	ASSERT(ps_id < (1<<ps_id_bit_width));
	qword |= ps_id;

	qword <<= state_id_bit_width;
	u64 state_id = m_state->get_id();
	ASSERT(state_id < (1<<state_id_bit_width));
	qword |= state_id;

// 	qword <<= tex_list_id_bit_width;
// 	u64 tlist_id = m_tex_list->get_id();
// 	ASSERT(tlist_id < (1<<tex_list_id_bit_width));
// 	qword |= tlist_id;

	return qword;
}


} // namespace render 
} // namespace xray 


#endif // #ifndef RES_EFFECT_H_INCLUDED
