////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "window_ide.h"
#include "window_view.h"
#include "project_tab.h"
#include "window_tab.h"
#include "object_inspector_tab.h"
#include "control_properties_tab.h"
#include "tools_tab.h"
#include "editor_world.h"
#include "property_holder.h"
#include "project.h"
#include "object_base.h"
#include "sound_editor.h"
#include "particle_editor.h"
#include "resource_editor.h"
#include "options_page_general.h"
#include "options_page_input.h"

#pragma managed(push,off)
#include <xray/input/mouse.h>
#include <xray/input/keyboard.h>
#include <xray/collision/api.h>
#include <xray/collision/space_partitioning_tree.h>
#include <xray/render/base/world.h>
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/editor_renderer.h>
#include <xray/editor/world/engine.h>
#include <xray/fs_path.h>
#include <xray/sound/world.h>
#include "picker.h"
#include "collision_object.h"
#pragma managed(pop)

#include "input_actions.h"
#include "input_engine.h"

#include "actions.h"
#include "action_engine.h"
#include "gui_binder.h"

#include "level_editor.h"
#include <xray/editor/base/images/images16x16.h>

#include "texture_document_factory.h"
#include "texture_editor_source.h"

using xray::math::int2;
using xray::collision::space_partitioning_tree;

namespace xray{
namespace editor{

void register_texture_cook();
void unregister_texture_cook();

namespace editor_resource_manager
{
	void initialize();
	void finalize();
}

editor_world::editor_world					( xray::editor::engine& engine ) :
	m_engine					( engine ),
	m_viewport_size				( 800, 600 ),
	m_active_action				( NULL ),
	m_collision_tree			( NULL ),
	m_editor_mode				( true ),
	m_paused					( false ),
	m_active_editor				( nullptr ),
	m_b_draw_grid				( true ),
	m_b_draw_collision			( false ),
	m_near_far_plane			( 0.2f, 500.0f ),
	m_object_focus_distance		( 20.0f )
{
	register_texture_cook		();
	editor_resource_manager::initialize();
	m_command_engine			= gcnew xray::editor_base::command_engine( 100 );
	m_tool_windows.initialize	(this);
	m_editors					= gcnew editors_list;
	m_window_ide				= gcnew window_ide( *this );

	m_camera_props.view_point_distance		= 70.7f;
	m_camera_props.view_point_distance_min	= 2;
	m_camera_props.view_move_z_coeff		= 1000;
	m_camera_props.view_move_speed_factor	= 2;

	set_view_matrix(		math::create_camera_direction(
								float3( 16.f, 10.f, 0.f ),
								float3( -1.f,  -1.f, 0.f ).normalize( ),
								float3( 0.f,  1.f, 0.f ) 
								) );

	m_projection			= 	
		math::create_projection( math::pi_d4, 1/(4.f/3.f), m_near_far_plane.x, m_near_far_plane.y );


	m_picker			= NEW (picker)( this );
	m_flying_camera		= NEW (flying_camera)	( this );

	m_input_engine		= gcnew editor::input_engine( *this );
	m_action_engine		= gcnew editor::action_engine( m_input_engine );
	
	// initializes gui binding to actions
	m_gui_binder		= gcnew gui_binder( action_engine() ); 
	register_actions	( );

	load_editors			();

	m_input_engine->load	();

	ide()->get_options_manager()->register_page( "General", gcnew options_page_general( *this ) );
	ide()->get_options_manager()->register_page( "General\\Input", gcnew options_page_input( *this ) );

}

editor_world::~editor_world					(  )
{
	for(int i=0; i<m_editors->Count; ++i)
		m_editors->default[i]->destroy( );

	m_editors->Clear				( );

	collision::destroy				(m_collision_tree);
	delete							m_action_engine;
	delete							m_input_engine;

	delete					m_command_engine;
	DELETE					(m_picker);
	DELETE					(m_flying_camera);
	//unregister_texture_cook	();
	editor_resource_manager::finalize();
}

bool editor_world::close_query( )
{
	for(int i=0; i<m_editors->Count; ++i)
	{
		if( !m_editors->default[i]->close_query() )
			return false;
	}
	return true;
}

void editor_world::on_window_closing				( )
{
	save_settings					( );
	clear_resources					( );

	if(m_tool_windows.m_texture_editor.operator->() != nullptr)
		m_tool_windows.m_texture_editor->manual_close	();

	m_tool_windows.m_particle_editor->manual_close	();
}

void editor_world::load_settings( )
{
	for(int i=0; i<m_editors->Count; ++i)
	{
		xray::editor_base::editor_base^ e	= m_editors->default[i];
		e->load_settings					();
	}

	RegistryKey^ product_key	= ide()->base_registry_key();
	RegistryKey^ editor_key 	= get_sub_key(product_key, "general");

	m_b_draw_grid				= System::Convert::ToBoolean(editor_key->GetValue("draw_grid", true));
	m_b_draw_collision			= System::Convert::ToBoolean(editor_key->GetValue("draw_collision", false));

	m_near_far_plane.x			= System::Convert::ToSingle(editor_key->GetValue("near_plane", 0.2f));
	m_near_far_plane.y			= System::Convert::ToSingle(editor_key->GetValue("far_plane", 500.0f));

	m_object_focus_distance		= System::Convert::ToSingle(editor_key->GetValue( "focus_distance", 20.0f));

	editor_key->Close			();
	product_key->Close			();
}

void editor_world::save_settings( )
{
	m_input_engine->save		();

	for(int i=0; i<m_editors->Count; ++i)
		m_editors->default[i]->save_settings( );

	RegistryKey^ product_key	= ide()->base_registry_key();
	RegistryKey^ editor_key		= get_sub_key(product_key, "general" );

	editor_key->SetValue		( "draw_grid", m_b_draw_grid );
	editor_key->SetValue		( "draw_collision", m_b_draw_collision );

	editor_key->SetValue		( "near_plane", m_near_far_plane.x );
	editor_key->SetValue		( "far_plane", m_near_far_plane.y );

	editor_key->SetValue		( "focus_distance", m_object_focus_distance );

	editor_key->Close			( );
	product_key->Close			( );
}

void editor_world::load( )
{
	g_allocator->user_current_thread_id( );

	m_collision_tree = &(*(collision::create_space_partitioning_tree( g_allocator, 1.f, 1024 )));

	for(int i=0; i<m_editors->Count; ++i)
	{
		xray::editor_base::editor_base^ e	= m_editors->default[i];
		e->initialize		( );
	}

	load_settings			( );

	m_window_ide->load_contents_layout	( );
	m_window_ide->Show					( );
}

void editor_world::run			( ) 
{
	System::Windows::Forms::Application::Run			( m_window_ide );
}

void editor_world::clear_resources	( )
{
	for(int i=0; i<m_editors->Count; ++i)
	{
		xray::editor_base::editor_base^ e	= m_editors->default[i];
		e->clear_resources		();
	}
}

HWND editor_world::main_handle		( )
{
	System::Int32	result = m_window_ide->Handle.ToInt32( );
	return			( *( HWND* )&result  );
}

HWND editor_world::view_handle		( )
{
	System::Int32	result = m_window_ide->view().draw_handle().ToInt32( );
	return			( *( HWND* )&result  );
}


void editor_world::tick			( )
{
	if(!engine().app_is_active())
	{
		resources::dispatch_callbacks();
		return;
	}

	if( !m_engine.on_before_editor_tick	( ) )
		return;

	resources::dispatch_callbacks();


	m_flying_camera->tick		( );

	if( m_editor_mode )
	{
		execute_input			( );

		for(int i=0; i<m_editors->Count; ++i)
			m_editors->default[i]->tick( );

	}
	m_gui_binder->update_items			();

	ide()->get_view_size(m_viewport_size);

	if( m_editor_mode )
	{
		get_renderer().set_view_matrix( math::invert4x3( m_inverted_view ) );
		engine().get_sound_world().set_listener_properties( m_inverted_view, g_allocator );

		m_projection			= 	
			math::create_projection( math::pi_d4, 1/((float)m_viewport_size.x/(float)m_viewport_size.y), m_near_far_plane.x, m_near_far_plane.y );
		
		get_renderer().set_projection_matrix( m_projection );

		float3 p, d;
		get_camera_props			(p, d);
		System::String^ cam_props	= System::String::Format("Camera: {0:f2} {1:f2} {2:f2}", p.x, p.y, p.z);
		ide()->set_status_label		(0, cam_props);
	}

	// Perform editor rendering.
	render							( 0, xray::math::float4x4() );

	// Process rendering commands and draw the frame 
 	//***m_engine.run_renderer_commands	( );
 	//***m_engine.draw_frame				( );
	
	//if( m_editor_mode )
	get_renderer().draw_frame		( );
	engine().on_after_editor_tick	( );
	//m_engine.get_renderer_world().game().draw_frame				( );

}

struct render_predicate : private boost::noncopyable {
	inline			render_predicate( xray::render::debug::renderer& renderer ) :
		m_renderer					( renderer )
	{
	}

	inline	void	operator ( )	( xray::collision::object const* const object ) const
	{
		ASSERT						( object );
		object->render				( m_renderer );
	}

	xray::render::debug::renderer&	m_renderer;
}; // struct render_predicate


void editor_world::render			( u32 frame_id, xray::math::float4x4 view_matrix )
{
	XRAY_UNREFERENCED_PARAMETERS( frame_id, view_matrix );

// 	xray::vectora< float2 > vec( g_allocator );
// 	vec.push_back( float2( 400, 400 ) );
// 	vec.push_back( float2( get_mouse_xy().x+m_viewport_width/2, get_mouse_xy().y+m_viewport_height/2 ) );
//	get_renderer().draw_screen_lines( vec, math::color_xrgb( 255, 255, 255 ), 0xCCCCCCCC );
// 	for( unsigned int i = 0; i<  m_tris.size(); i+=3 )
// 	{
// 		get_renderer().draw_triangle( m_tris[i], m_tris[i+1], m_tris[i+2], 0xFFFFFFFF );
// 	}
// 	get_renderer().flush();

	if( m_editor_mode )
	{
		//*************m_engine.flush_debug_commands	( );

		if(m_b_draw_grid)
			render_grid						( );

		if(m_b_draw_collision) {

			render_predicate	predicate( engine().get_renderer_world().editor().debug() );

			get_collision_tree()->for_each	( collision::space_partitioning_tree::predicate_type(&predicate, &render_predicate::operator()) );
		}

		for(int i=0; i<m_editors->Count; ++i)
			m_editors->default[i]->on_render( );


		m_tool_windows.on_render		( engine().get_renderer_world() );
		//*************m_engine.run_renderer_commands	( );
	}
}

void editor_world::on_idle_start	( )	
{
}

void editor_world::on_idle			( )
{
}

void editor_world::on_idle_end		( )
{
}

bool editor_world::execute_input					( )
{
	m_input_engine->execute();

	return true; 
}

void editor_world::get_camera_props( float3& p, float3& d)
{
	p	= get_inverted_view_matrix().c.xyz();
	d	= get_inverted_view_matrix().k.xyz();
}

void editor_world::set_camera_props( const float3& p, const float3& d)
{
	float4x4 m		= math::create_camera_direction( p, d, float3(0.f, 1.f, 0.f ) );
	set_view_matrix	( m );
}

void editor_world::set_view_matrix	( float2 const& raw_angles, float const forward, float const right, float const up )
{
	float4x4 const view_inverted = get_inverted_view_matrix( );
	float const x_new			= 
		math::clamp_r			(
		math::angle_normalize_signed( view_inverted.get_angles_xyz_INCORRECT( ).x + raw_angles.x ),
		-math::pi -math::pi_d2 + math::deg2rad( 1.f ),
		math::pi + math::pi_d2 - math::deg2rad( 1.f )
		);
	float3 const new_angles		= float3( x_new, view_inverted.get_angles_xyz_INCORRECT( ).y + raw_angles.y, view_inverted.get_angles_xyz_INCORRECT( ).z );
#if 1
	math::float4x4 rotation		= math::create_rotation_INCORRECT( new_angles );
#else // #if 1
	// Rotation matrix to rotate around X axis of the camera.
	math::float4x4	rotation_X_camrea		= math::create_rotation( view_inverted.i.xyz(), -raw_angles.x );

	// Rotation matrix to rotate around Y axis of the editor_world.
	math::float3 const	angles_Y			= math::float3(  0, raw_angles.y*(view_inverted.j.xyz().y/abs(m_view_saved.j.xyz().y)), 0 );
	math::float4x4	rotation_Y_world		= math::create_rotation( angles_Y );

	math::float4x4 rotation			= rotation_X_camrea*rotation_Y_world;
#endif // #if 1


	float3 const position		= 
		view_inverted.c.xyz( ) +
		view_inverted.i.xyz( )*right +
		view_inverted.j.xyz( )*up +
		view_inverted.k.xyz( )*forward;
	math::float4x4 const translation	= math::create_translation( position );
	set_view_matrix(	math::invert4x3 ( rotation * translation ));
}

float3 editor_world::screen_to_3d		( float2 const& screen, float z)
{
	float3 result( screen.x, screen.y, z);

	float4x4 matrix = get_projection_matrix();

	result.x *= ( z/matrix.e00/(m_viewport_size.x/2));
	result.y *= ( z/matrix.e11/(m_viewport_size.y/2));

	return result;
}
float3 editor_world::screen_to_3d_world( float2 const& screen, float z)
{
	return get_inverted_view_matrix().transform_position( screen_to_3d ( screen, z) );
}

camera_properties& editor_world::get_camera_props	( )
{
	return m_camera_props;
}

space_partitioning_tree*	editor_world::get_collision_tree	( )
{
	return m_collision_tree;
}

void editor_world::get_mouse_ray ( float3& origin, float3& direction )
{
	// Get mouse current position
	System::Drawing::Point mouse_position;
	ide()->get_mouse_position(mouse_position);

	get_screen_ray ( int2( mouse_position.X, mouse_position.Y), origin, direction );
}

void editor_world::get_screen_ray ( int2 screen_xy, float3& origin, float3& direction )
{
	direction				= screen_to_3d ( float2( (float)screen_xy.x, (float)-screen_xy.y), 1 );
	direction				= get_inverted_view_matrix().transform_direction( direction );
	direction.normalize		();
	origin					= get_inverted_view_matrix().c.xyz();
}

xray::editor::picker& editor_world::get_picker		( )
{
	return *m_picker;		
}

xray::math::int2 editor_world::get_mouse_position					( )
{
	xray::math::int2 result;
	ide()->get_mouse_position(result);
	return			result; 
}

math::int2	editor_world::get_viewport			( )
{
	return m_viewport_size;
}

int editor_world::rollback		( int steps )
{
	return m_command_engine->rollback( steps );
}

bool editor_world::undo_stack_empty( )
{
	return m_command_engine->undo_stack_empty();
}

bool editor_world::redo_stack_empty( )
{
	return m_command_engine->redo_stack_empty();
}

enum_editor_cursors editor_world::get_view_cursor			( )
{
	return ide()->view().get_cursor();
}

enum_editor_cursors editor_world::set_view_cursor			( enum_editor_cursors cursor )
{
	return ide()->view().set_cursor( cursor );
}

void editor_world::render_grid				( )
{
	u32	const	line_count	= 200;
	float const	step		= line_count*0.5f;
	for ( u32 i=0; i<=line_count; ++i)
		if( i%10 > 0)
			engine().get_renderer_world().editor().draw_line	( float3( -step + i, 0.f, -step ), float3( -step + i, 0.f, step ), math::color_xrgb ( 64, 64, 64 ) );
		else
			engine().get_renderer_world().editor().draw_line	( float3( -step + i, 0.f, -step ), float3( -step + i, 0.f, step ), math::color_xrgb ( 164, 164, 164 ) );

	for ( u32 i=0; i<=line_count; ++i)
		if( i%10 > 0)
			engine().get_renderer_world().editor().draw_line	( float3( -step, 0.f, -step + i ), float3( step, 0.f, -step + i ), math::color_xrgb ( 64, 64, 64 ) );
		else
			engine().get_renderer_world().editor().draw_line	( float3( -step, 0.f, -step + i ), float3( step, 0.f, -step + i ), math::color_xrgb ( 164, 164, 164 ) );
}

void editor_world::show_sound_editor( )
{
	sound_editor^ tab		= m_tool_windows.m_sound_editor;
	tab->Show				();
	tab->Focus				();
}

void editor_world::show_dialog_editor( )
{
	xray::editor_base::editor_base^ ed = get_editor_by_name("dialog_editor");
	if(ed==nullptr)
	{
		dialog_editor::editor_creator::memory_allocator(g_allocator);
		ed = dialog_editor::editor_creator::create_editor(gcnew System::String(engine().get_resource_path()));
		m_editors->Add(ed);
	}
	ed->show();
}

void editor_world::show_particle_editor()
{
	particle_editor ^ tab	= m_tool_windows.m_particle_editor;
	if(tab->WindowState == System::Windows::Forms::FormWindowState::Minimized)
		tab->WindowState = System::Windows::Forms::FormWindowState::Normal;
	tab->Show				();
	tab->Focus				();
}

void editor_world::show_texture_editor()
{
	m_tool_windows.m_texture_editor = gcnew resource_editor(*this, gcnew texture_document_factory(), gcnew texture_editor_source());
	m_tool_windows.m_texture_editor->name = "texture_editor";
	resource_editor ^ tab			= m_tool_windows.m_texture_editor;
	tab->Show						();
	tab->properties_panel_caption	= "Texture Properties";
	tab->view_panel_caption			= "Textures";
	tab->Text						= "Texture Editor";
	tab->Focus						();
}

void editor_world::on_active_control_changed( xray::editor::editor_control_base^ c )
{
	if(c)
		set_view_cursor( enum_editor_cursors::enum_editor_cursors_dragger );
	else
		set_view_cursor( enum_editor_cursors::enum_editor_cursors_default );

	//m_tool_windows.m_control_properties_tab->show_properties(c);
	//ide()->show_tab			(m_tool_windows.m_control_properties_tab);
}

void editor_world::register_actions( )
{
	// Maya style mouse actions 
	m_action_engine->register_action( gcnew	mouse_action_view_move_xy		( "view maya move xy",	this ), "Alt+MButton" );
	m_action_engine->register_action( gcnew	mouse_action_view_move_z		( "view maya move z",	this ), "Alt+RButton" );
	m_action_engine->register_action( gcnew	mouse_action_view_rotate_around	( "view maya rotate", this ), "Alt+LButton" );
	
	// Old LE style mouse actions 
	m_action_engine->register_action( gcnew	mouse_action_view_rotate		( "view LE rotate", this ), "Space+LButton+RButton" );
	m_action_engine->register_action( gcnew	mouse_action_view_move_y_reverse( "view LE move y", this ), "Space+RButton" );
	m_action_engine->register_action( gcnew	mouse_action_view_move_xz_reverse( "view LE move xz",this ), "Space+LButton" );

	// Transform controls mouse actions
	
	m_action_engine->register_action( gcnew action_editor_mode	( "switch editor mode",	this ));
	m_action_engine->register_action( gcnew action_pause		( "pause",	this ));

	// Editing actions
	m_action_engine->register_action( gcnew action_undo		( "undo",	this ), "Control+Z" );
	m_action_engine->register_action( gcnew action_redo		( "redo",	this ), "Control+Y" );

	m_action_engine->register_action( gcnew action_show_sound_editor	( "sound editor",	this ));
	m_action_engine->register_action( gcnew action_show_dialog_editor	( "dialog editor",	this ));
	m_action_engine->register_action( gcnew action_show_particle_editor	( "particle editor",	this ));
	m_action_engine->register_action( gcnew action_show_texture_editor	( "texture editor",	this ));

	// and bind to ui elements
	ide()->add_menu_item(m_gui_binder, "undo",		"EditMenuItem", 1);
	ide()->add_menu_item(m_gui_binder, "redo",		"EditMenuItem", 2);

	ide()->register_image("switch editor mode", xray::editor_base::mode_editor );
	ide()->register_image("pause", xray::editor_base::mode_pause );

	ide()->add_button_item("MainToolStrip", 0);
	ide()->add_button_item(m_gui_binder, "switch editor mode", "MainToolStrip", 1);
	ide()->add_button_item(m_gui_binder, "pause", "MainToolStrip", 2);

	ide()->add_menu_item(m_gui_binder, "particle editor",	"ToolsMenuItem", 1);
	ide()->add_menu_item(m_gui_binder, "dialog editor",		"ToolsMenuItem", 2);
	ide()->add_menu_item(m_gui_binder, "sound editor",		"ToolsMenuItem", 3);
	ide()->add_menu_item(m_gui_binder, "texture editor",	"ToolsMenuItem", 4);
}

IDockContent^ editor_world::find_dock_content(System::String^ type_name)
{
	IDockContent^	result = m_tool_windows.get_by_typename(type_name);
	if(result==nullptr)
	{
		for(int i=0; i<m_editors->Count; ++i)
		{
			result = m_editors->default[i]->find_dock_content(type_name);
			if(result)
				break;
		}
	}
	return result;
}

void editor_world::pause(bool bpause)
{
	m_paused = bpause;
}

void editor_world::editor_mode(bool beditor_mode)
{

	if( beditor_mode ) 
	{
		engine().unload_level	( );

		engine().enable_game	( false );
		
		while ( ShowCursor(TRUE) <= 0 );

		ide()->view().unclip_cursor();

	}else
	{
		editor_base::editor_base^ ed	= m_active_editor;
		level_editor^ le				= safe_cast<level_editor^>(ed);
		le->save_internal				( ); 

		engine().enable_game					( true );
		fs::path_string project_resource_name	= le->get_project()->resource_name();
		engine().load_level						( project_resource_name.c_str() );

		ide()->view().clip_cursor		( );

		while ( ShowCursor(FALSE) >= 0 );
	}

	m_editor_mode						= beditor_mode;
}

void editor_world::load_editors()
{
	level_editor^ le	= gcnew level_editor(*this);
	m_editors->Add		(le);
	m_active_editor		= le;
}

void editor_world::set_default_layout	( )
{
	if ( !m_active_editor )
		return;

	m_active_editor->set_default_layout	( );
}

xray::editor_base::editor_base^ editor_world::get_editor_by_name	( System::String^ editor_name )
{
	for(int i=0; i<m_editors->Count; ++i)
	{
		if(m_editors->default[i]->name()==editor_name)
			return m_editors->default[i];
	}

	return nullptr;
}

} // namespace editor
} // namespace xray
