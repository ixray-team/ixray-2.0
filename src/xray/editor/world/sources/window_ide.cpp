////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "window_ide.h"
#include "window_view.h"
#include "window_tab.h"
#include "project_tab.h"
#include "tools_tab.h"
#include "project.h"
#include "editor_world.h"
#include "gui_binder.h"
#include "about_dialog.h"
#include "loading_dialog.h"
#include "action_engine.h"
#include "input_engine.h"
#include "sound_editor.h"
#include "particle_editor.h"
#include "options_dialog.h"
#include <xray/editor/base/images/images16x16.h>
#include "level_editor.h"
#include <xray/editor/base/images_loading.h>

using namespace System::Windows::Forms;
using namespace WeifenLuo::WinFormsUI::Docking;

using xray::editor::window_ide;

void install_hooks();
void remove_hooks();

void window_ide::in_constructor	( )
{
	m_in_idle					= 0;
	Application::Idle			+= gcnew System::EventHandler(this, &window_ide::on_idle);
	
	m_view						= gcnew window_view( *this );
	m_view->m_input_key_holder.subscribe_on_changed( gcnew keys_combination_changed(this, &window_ide::key_combination_changed));
	m_window_rectangle = gcnew Drawing::Rectangle(Location,Size);

	// Initialize options dialog
	m_options_dialog			= gcnew options_dialog();
	m_images16x16				= xray::editor_base::image_loader::load_images( "images16x16", 16, safe_cast<int>(xray::editor_base::images16x16_count), GetType());
	install_hooks				();
}

void window_ide::custom_finalize( )
{
	Application::Idle			-= gcnew System::EventHandler(this, &window_ide::on_idle);
	remove_hooks				();
}

void window_ide::load_contents_layout( )
{
	bool first_time					=
		!controls::serializer::deserialize_dock_panel_root(
			this,
			Editor,
			"editor",
			gcnew DeserializeDockContent(this, &window_ide::reload_content)
		);
	
	if ( first_time ) {
		Width				= 800;
		Height				= 600;
		m_window_rectangle	= gcnew Drawing::Rectangle(Location,Size);

		m_view->Show		( Editor, WeifenLuo::WinFormsUI::Docking::DockState::Document );
		get_editor_world().set_default_layout	( );

		WindowState			= FormWindowState::Maximized;
	}
}

void window_ide::on_idle		( System::Object ^sender, System::EventArgs ^event_args )
{
	XRAY_UNREFERENCED_PARAMETERS( sender, event_args );

	if ( !m_in_idle++ )
		m_editor_world.on_idle_start( );

	MSG message;
	do {
		m_editor_world.tick			( );
		m_view->on_idle				( );
		m_editor_world.on_idle		( );
	}
	while ( !PeekMessage( &message, HWND( 0 ), 0, 0, 0 ) );

	if ( !--m_in_idle )
		m_editor_world.on_idle_end		( );

}

void window_ide::window_ide_FormClosing		(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETER	( sender );

	if( !m_editor_world.close_query ( ) )
	{
		e->Cancel = true;
		return;
	}

	controls::serializer::serialize_dock_panel_root(this, Editor, "editor");
	m_editor_world.on_window_closing					( );
}


void window_ide::window_ide_LocationChanged	(System::Object^  sender, System::EventArgs^  e)
{
	m_view->window_view_LocationChanged	(sender, e);

	if (WindowState == System::Windows::Forms::FormWindowState::Maximized)
		return;

	if (WindowState == System::Windows::Forms::FormWindowState::Minimized)
		return;
	
	m_window_rectangle					= gcnew Drawing::Rectangle(Location,Size);
}

void window_ide::window_ide_SizeChanged		(System::Object^  sender, System::EventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETERS( sender, e );

	if (WindowState == System::Windows::Forms::FormWindowState::Maximized)
		return;

	if (WindowState == System::Windows::Forms::FormWindowState::Minimized)
		return;
	
	m_window_rectangle					= gcnew Drawing::Rectangle(Location,Size);
}

void window_ide::add_tab				( System::String^ tab_id, xray::editor::property_holder& property_holder )
{
	window_tab^	tab						= gcnew window_tab( tab_id, property_holder );
	tab->Show							( Editor, WeifenLuo::WinFormsUI::Docking::DockState::DockLeft );
}

void window_ide::add_window	( WeifenLuo::WinFormsUI::Docking::DockContent^ form, WeifenLuo::WinFormsUI::Docking::DockState dock )
{
	form->Show( Editor, dock);
}

void window_ide::window_ide_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETER	( sender );

	if (e->Alt && e->KeyCode == System::Windows::Forms::Keys::Return) 
	{
		bool current	= get_editor_world().editor_mode();
		get_editor_world().editor_mode(!current);
		e->Handled		= true;
		return;
	}

	m_view->m_input_key_holder.register_key_event( e, true );
}

void window_ide::key_combination_changed( System::String^ combination )
{
	System::String^ status	= "Keys: ["+combination+"]";
	set_status_label		(3, status );
}

void window_ide::window_ide_KeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETER	( sender );

	m_view->m_input_key_holder.register_key_event( e, false );
}

ArrayList^ window_ide::get_held_keys( )
{
	return m_view->m_input_key_holder.get_held_keys();
}

int	window_ide::get_current_keys_string( System::String^& combination )
{
	return m_view->m_input_key_holder.get_current_keys_string( combination );
}

Point window_ide::get_mouse_pos_infinte( )
{
	return m_view->m_mouse_real;
}

void window_ide::get_mouse_position(xray::math::int2& value )
{
	System::Drawing::Point pt = m_view->get_mouse_position();
	value.set(pt.X, pt.Y);
}

void window_ide::get_mouse_position(System::Drawing::Point% value)
{
	value = m_view->get_mouse_position();
}

void window_ide::set_mouse_position(System::Drawing::Point value)
{
	m_view->set_mouse_position(value);
}

void window_ide::get_view_size(xray::math::int2& value)
{
	System::Drawing::Size pt = m_view->get_view_size();
	value.set(pt.Width, pt.Height);
}

void window_ide::get_view_size(System::Drawing::Size% value)
{
	value = m_view->get_view_size();
}

Void window_ide::exitToolStripMenuItem_Click	(System::Object^, System::EventArgs^)
{
	Close						();
}

Void window_ide::aboutToolStripMenuItem_Click(System::Object^, System::EventArgs^)
{
	about_dialog^ about			= gcnew about_dialog();
	about->ShowDialog			(this);
}

Void window_ide::optionsToolStripMenuItem_Click(System::Object^, System::EventArgs^)
{
	m_editor_world.input_engine()->stop		();
	m_options_dialog->ShowDialog			(this);
	m_editor_world.input_engine()->resume	();
}

void window_ide::set_mouse_sensivity( float sens )
{
	extern float g_mouse_sensitivity;	
	g_mouse_sensitivity		= sens;
}

void window_ide::show_tab(WeifenLuo::WinFormsUI::Docking::DockContent^ tab)
{
	show_tab		( tab, WeifenLuo::WinFormsUI::Docking::DockState::Float );
}

void window_ide::show_tab	(WeifenLuo::WinFormsUI::Docking::DockContent^ tab, WeifenLuo::WinFormsUI::Docking::DockState state)
{
	if ( tab->DockPanel ) {
		tab->Show	(Editor);
		return;
	}

	tab->Show		(Editor, state );
}

using System::Windows::Forms::ToolStripMenuItem;
using System::Windows::Forms::ToolStripButton;

bool window_ide::is_mouse_in_view()
{
	return m_view->m_mouse_in_view;
}

ref class menu_item_comparer : public System::Collections::Generic::IComparer<ToolStripItem^>
{
public:
	virtual int Compare	(ToolStripItem^ x, ToolStripItem^ y){
//		System::Collections::Comparer^ c = System::Collections::Comparer::Default;
//		return c->Compare(2,4);
		return System::Collections::Comparer::Default->Compare((int)(x->Tag), (int)(y->Tag));
	}
};

void add_item_to_collection(ToolStripItemCollection^ collection, ToolStripItem^ item, int item_prio)
{
	item->Tag = item_prio;
	for each(ToolStripItem^ itm in collection)
	{
		int prio = System::Convert::ToInt32(itm->Tag->ToString());
		if(prio > item_prio)
		{
			int idx = collection->IndexOf(itm);
			collection->Insert(idx, item);
			return;
		}
	}
	collection->Add(item); // add to tail
}

ToolStripMenuItem^ window_ide::get_parent_menu_item(System::String^ key)
{
	ToolStripMenuItem^ item		= nullptr;
	int idx = menuStrip->Items->IndexOfKey(key);
	ASSERT(idx!=-1, "menu item not found %s", unmanaged_string(key).c_str());
	item						= safe_cast<ToolStripMenuItem^>(menuStrip->Items[idx]);
	return						item;
}

void window_ide::add_menu_item(System::String^ key, System::String^ text, int prio)
{
	int idx = menuStrip->Items->IndexOfKey(key);
	ASSERT_U					(idx==-1, "menu item already exist %s", unmanaged_string(key).c_str());

	ToolStripMenuItem^ menu_item	= nullptr;
	menu_item					= gcnew ToolStripMenuItem;
	menu_item->Name				= key;
	menu_item->Text				= text;
	add_item_to_collection(menuStrip->Items, menu_item, prio);
}

void window_ide::add_button_item(System::String^ key, int prio)
{
	m_view->CreateToolStrip(key, prio);
}

void window_ide::add_menu_item(xray::editor::gui_binder^ binder, System::String^ action_name, System::String^ key, int prio)
{
	ToolStripMenuItem^ parent_menu_item = get_parent_menu_item(key);

	ToolStripMenuItem^ menu_item		= gcnew ToolStripMenuItem;
	add_item_to_collection				(parent_menu_item->DropDownItems, menu_item, prio);
	parent_menu_item->DropDownOpening	-= gcnew System::EventHandler(binder, &gui_binder::gui_menu_drop_down);
	parent_menu_item->DropDownOpening	+= gcnew System::EventHandler(binder, &gui_binder::gui_menu_drop_down);
	binder->bind						(menu_item, action_name);
}

void window_ide::add_button_item(xray::editor::gui_binder^ binder, System::String^ action_name, System::String^ key, int prio)
{
	XRAY_UNREFERENCED_PARAMETER(prio);
	ToolStrip^ strip			= m_view->GetToolStrip(key);
	ToolStripButton^ button		= nullptr;
	int idx = strip->Items->IndexOfKey(action_name);
	ASSERT_U					(idx==-1, "button already exist %s", unmanaged_string(action_name).c_str());

	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);

	binder->bind				(button, action_name);
	button->Image				= get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;
}

void window_ide::register_image(System::String^ key, xray::editor_base::images16x16 image_id )
{
	m_images[key]				= m_images16x16->Images[image_id];
}

System::Drawing::Image^ window_ide::get_image(System::String^ key)
{
	if(m_images.Contains(key))
		return safe_cast<System::Drawing::Image^>(m_images[key]);
	else
		return nullptr;
}

void window_ide::FileMenuItem_DropDownOpening(System::Object^, System::EventArgs^)
{
	recentFilesToolStripMenuItem->DropDownItems->Clear();
	level_editor^ ed = dynamic_cast<level_editor^>(m_editor_world.get_editor_by_name("level_editor"));

	if(ed)
	{
		System::Collections::ArrayList recent_list;
		ed->get_recent_list(%recent_list);
		for each(System::String^ filename in recent_list)
		{
			System::Windows::Forms::ToolStripItem^ item = recentFilesToolStripMenuItem->DropDownItems->Add(filename);
			item->Click += gcnew System::EventHandler(this, &window_ide::RecentFileMenuItem_Click);
		}
	}
}

void window_ide::RecentFileMenuItem_Click(System::Object^ sender, System::EventArgs^)
{
	System::Windows::Forms::ToolStripItem^ item = safe_cast<System::Windows::Forms::ToolStripItem^>(sender);

	xray::editor_base::editor_base^ active_editor			= m_editor_world.get_active_editor();
	active_editor->load						(item->Text);
}

void window_ide::set_status_label(int idx, System::String^ text)
{
	switch (idx)
	{
	case 0:	
		toolStripStatusLabel0->Text = text;
		break;
	case 1:
		toolStripStatusLabel1->Text = text;
		break;
	case 2:
		toolStripStatusLabel2->Text = text;
		break;
	case 3:
		toolStripStatusLabel3->Text = text;
		break;
	}
}

xray::editor::options_manager^ window_ide::get_options_manager() 
{
	return m_options_dialog;
}

void window_ide::WndProc(Message% m)
{
	super::WndProc(m);

    if (m.Msg == WM_ACTIVATE)
		OnWM_Activate((int)m.WParam, (int)m.LParam);
}

void window_ide::OnWM_Activate(WPARAM wParam, LPARAM lParam)
{
	XRAY_UNREFERENCED_PARAMETER		(lParam);
	u16 fActive						= LOWORD(wParam);
	BOOL fMinimized					= (BOOL) HIWORD(wParam);
	bool bActive					= ((fActive!=WA_INACTIVE) && (!fMinimized)) ? true : false;

	if (bActive != m_editor_world.engine().app_is_active())
	{
		if (bActive)	
		{
			// activate
			m_view->window_view_Activated		();
			m_editor_world.engine().on_app_activate();
		}else	
		{
			// deactivate
			m_view->window_view_Deactivate		();
			m_editor_world.engine().on_app_deactivate();
		}
	}
}
