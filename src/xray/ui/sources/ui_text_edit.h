////////////////////////////////////////////////////////////////////////////
//	Created		: 05.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#ifndef UI_TEXT_EDIT_H_INCLUDED
#define UI_TEXT_EDIT_H_INCLUDED

#include "ui_text.h"
#include "ui_text_edit_defs.h"

class base_edit_action;
struct undo_{
	pcstr text;
	u16	caret;
};
typedef xray::ui::vector<undo_>			history;
typedef history::iterator				history_it;

class ui_text_edit :	public ui::text_edit,
						public ui_text
{
	typedef ui_text		super;
public:
						ui_text_edit		(ui_world& w, enum_text_edit_mode mode);
	virtual				~ui_text_edit		();
	virtual void		draw				(xray::render::world& w);
	virtual void		tick				();

	virtual void		set_text			(pcstr text);
	virtual pcstr		get_text			() const						{return m_text_buf.c_str();};
	virtual ui::window*	w					()								{return this;}
	virtual ui::text*	text				()								{return this;}

			bool 		is_shift_state		(enum_shift_state const state) const;
			void		set_shift_state		(enum_shift_state const state, bool b_set);

// controls
			void		copy_to_clipboard	();
			void		cut_to_clipboard	();
			void		paste_from_clipboard();

			void		move_cursor_begin	();
			void		move_cursor_end		();
			void		move_cursor_left	();
			void		move_cursor_right	();

			void		undo				();
			void		redo				();
			void		commit_text			();

			void		insert_character	(char const ch);

			void		delete_selection	();
			void		delete_left			();
			void		delete_right		();

			void		reset_selection		();
			void		select_all			();
			void		switch_insert_mode	();

	virtual	u16			get_caret_position	() const							{return m_caret_pos;}
	virtual	void		set_caret_position	(u16 const pos, bool b_move=false);
private:
			void		init_internals		(enum_text_edit_mode mode);
			void		add_action			(base_edit_action* action);
			void		draw_highlighted_rect(xray::render::world& w);
			void		draw_cursor			(xray::render::world& w);

//--
			void		move_cursor			(enum_cursor_moving action);
			float		get_local_position	(u16 const pos) const;
			u16			calc_left_word_position	(u16 caret) const;
			u16			calc_right_word_position(u16 caret) const;
			
			bool		get_selection		()	const							{return m_sel_start!=m_sel_end;}
			bool		on_focus			(window* w, int p1, int p2);
			bool		on_keyb_action		(window* w, int p1, int p2);

	
	enum {edit_max_len=512};
	typedef xray::fixed_string<edit_max_len> editor_string;
	editor_string							m_text_buf;
	
	typedef xray::ui::vector<base_edit_action*>	edit_actions;
	typedef edit_actions::iterator			edit_actions_it;
	typedef edit_actions::const_iterator	edit_actions_cit;

	edit_actions							m_edit_actions;
	base_edit_action*						m_last_action;
	float									m_last_action_time;

	history									m_undo_history;
	history									m_redo_history;

	u16										m_caret_pos;
	u16										m_sel_start;
	u16										m_sel_end;
	shift_state								m_shift_state;

	bool									m_b_undo;
	bool									m_b_redo;
	bool									m_b_insert_mode;
};//class ui_text_edit
#endif //UI_TEXT_EDIT_H_INCLUDED