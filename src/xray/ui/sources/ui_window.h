////////////////////////////////////////////////////////////////////////////
//	Created		: 30.01.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#ifndef UI_WINDOW_H_INCLUDED
#define UI_WINDOW_H_INCLUDED

typedef xray::ui::vector<ui_event_handler>		ui_event_handler_list;
typedef ui_event_handler_list::iterator			ui_event_handler_it;
typedef ui_event_handler_list::const_iterator	ui_event_handler_cit;

class ui_world;

class ui_window :	public ui::window,
					private boost::noncopyable
{
public:
							ui_window			();
	virtual					~ui_window			();

			void			set_position		(float2 const& pos);
			float2 const&	get_position		()						const	{return m_position;}
			void			set_size			(const float2& size);
			float2 const&	get_size			()						const	{return m_size;}

			void			set_visible			(bool vis)						{m_b_visible = vis;}
			bool			get_visible			()						const	{return m_b_visible;}

	virtual void			draw				(xray::render::world& w);
	virtual void			tick				();

	virtual void			add_child			(window* w, bool adopt);
	virtual void			remove_child		(window* w);
	virtual void			remove_all_childs	();
	virtual window*			get_child			(u32 idx)				const	{return m_childs[idx];}
	virtual u32				get_child_count		()						const	{return m_childs.size();}

	virtual window*			get_parent			()						const	{return m_parent;}
	virtual void			set_parent			(window* w);
	virtual window*			get_root			()								{return (m_parent)?m_parent->get_root():this;};

	virtual bool			get_orphan			() const;
	virtual void			set_orphan			(bool val);
	
	virtual void			set_focused			(bool val);
	virtual bool			get_focused			() const						{return m_b_focused;}
	virtual bool			get_tab_stop		() const						{return m_b_tab_stop;}

	virtual void			subscribe_event		(enum_window_events ev, ui_event_handler handler);
	virtual void			unsubscribe_event	(enum_window_events ev, ui_event_handler handler);

protected:
	typedef xray::ui::vector<window*>			window_list;
	typedef window_list::iterator				window_list_it;
	typedef window_list::const_iterator			window_list_cit;

	typedef associative_vector<enum_window_events, ui_event_handler_list, xray::ui::vector>	event_manager;
	typedef event_manager::iterator													event_manager_it;
	typedef event_manager::const_iterator											event_manager_cit;

			bool			emit_event			(enum_window_events ev, window* w, int p1, int p2);
			bool			process_event		(enum_window_events ev, int p1, int p2);

	float2					m_position;
	float2					m_size;
	window*					m_parent;
	event_manager			m_event_manager; //temporary, make global ui-event manager
	bool					m_b_visible;
	bool					m_b_orphan;
	bool					m_b_focused;
	bool					m_b_tab_stop;
public:
	window_list				m_childs;
};

void client_to_screen(const ui_window& w, float2& src_and_dest);

#include "ui_window_inline.h"

#endif //UI_WINDOW_H_INCLUDED