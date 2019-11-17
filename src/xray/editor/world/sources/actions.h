////////////////////////////////////////////////////////////////////////////
//	Created		: 14.05.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GUI_ACTIONS_H_INCLUDED
#define GUI_ACTIONS_H_INCLUDED

#include "action_single.h"

namespace xray {	
namespace editor {

ref class window_ide;
ref class input_engine;
ref class level_editor;
class editor_world;


//////////////////////////////////////////////////////////////////////////

public delegate void execute_delegate_managed();
public delegate bool enabled_delegate_managed();
public delegate bool checked_delegate_managed();

typedef fastdelegate::FastDelegate< void () >	execute_delegate_unmanaged;
typedef fastdelegate::FastDelegate< bool () >	enabled_delegate_unmanaged;

//////////////////////////////////////////////////////////////////////////
public ref class action_delegate : action_single
{
public:
					action_delegate ( System::String^ name, execute_delegate_managed^ execute_del );
					action_delegate ( System::String^ name, execute_delegate_unmanaged* execute_del );
	virtual			~action_delegate ();
	
	void			set_enabled(enabled_delegate_managed^ d );
	void			set_enabled(enabled_delegate_unmanaged* d );
	
	void			set_checked(checked_delegate_managed^ execute_del );

	virtual void	execute	() override 
	{
		if( execute_delegate_man )
		{
			execute_delegate_man	();
			return;
		}

		(*execute_delegate_unman)	();
		return;
	}

	virtual bool	enabled	() override
	{
		if( enabled_delegate_man )
			return enabled_delegate_man ();

		if( enabled_delegate_unman )
			return (*enabled_delegate_unman) ();

		return true;
	}

	virtual xray::editor::checked_state checked_state () override
	{
		if(checked_delegate_man)
			return checked_delegate_man() ? checked_state::checked : checked_state::unchecked;
		else
			return checked_state::uncheckable;
	}

private:
	execute_delegate_managed^	execute_delegate_man;
	enabled_delegate_managed^	enabled_delegate_man;

	execute_delegate_unmanaged*	execute_delegate_unman;
	enabled_delegate_unmanaged* enabled_delegate_unman;

	checked_delegate_managed^	checked_delegate_man;
}; // class action_delegate

public ref class editor_world_action : public action_single
{
public:
					editor_world_action		(System::String^ name, editor_world* world);
	virtual void	execute					() override {ASSERT(0);};
protected:
	editor_world*	m_editor_world;
};

public ref class action_undo : public editor_world_action
{
public:
					action_undo				(System::String^ name, editor_world* world ):editor_world_action(name, world){};
	
	virtual void	execute					() override;
	virtual bool	enabled					() override;
}; // class action_undo

public ref class action_redo : public editor_world_action
{
public:
					action_redo				(System::String^ name, editor_world* world ):editor_world_action(name, world){};

	virtual void	execute					() override;
	virtual bool	enabled					() override;
}; // class action_redo

public ref class action_show_sound_editor : public editor_world_action
{
public:
					action_show_sound_editor(System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
}; // class action_show_sound_editor

public ref class action_show_dialog_editor : public editor_world_action
{
public:
					action_show_dialog_editor( System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
}; // class action_show_dialog_editor

public ref class action_show_particle_editor : editor_world_action
{
public:
					action_show_particle_editor( System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
}; // class action_show_particle_editor

public ref class action_show_texture_editor : editor_world_action
{
public:
					action_show_texture_editor( System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
}; // class action_show_texture_editor

public ref class action_pause : public editor_world_action
{
public:
					action_pause			(System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
	virtual xray::editor::checked_state checked_state () override;
}; // class action_pause

public ref class action_editor_mode : public editor_world_action
{
public:
					action_editor_mode		(System::String^ name, editor_world* world):editor_world_action(name, world){};

	virtual void	execute					() override;
	virtual xray::editor::checked_state checked_state () override;
}; // class action_pause

public ref class action_focus : action_single
{
public:
	action_focus( System::String^ name, level_editor^ le);

	virtual void	execute	() override;
	virtual bool	enabled	() override;

private:
			void	do_focus_selection		();
			void	do_focus_pick_collision	();

	level_editor^	m_level_editor;
	float			m_halo_factor;

}; // class action_focus_selection


//////////////////////////////////////////////////////////////////////////
public ref class action_select_editor_control : action_single
{
public:
	action_select_editor_control ( System::String^ name, level_editor^ le, System::String^ control_id );

	virtual void			execute			() override;
	virtual xray::editor::checked_state	checked_state	() override;

private:
	level_editor^		m_custom_editor;
	System::String^			m_control_id;

}; // class action_select_editor_control

public ref class action_drop_objects : action_single
{
public:
	action_drop_objects ( System::String^ name, level_editor^ le, bool from_top );

	virtual void	execute	() override;
	virtual bool	enabled	() override;

private:
	level_editor^			m_level_editor;
	bool					m_from_top;

}; // class action_drop_objects

} // namespace xray	
} // namespace editor

#endif // #ifndef GUI_ACTIONS_H_INCLUDED