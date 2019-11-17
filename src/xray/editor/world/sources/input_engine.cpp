////////////////////////////////////////////////////////////////////////////
//	Created		: 28.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "input_engine.h"
#include "editor_world.h"
#include "input_action.h"
#include <xray/configs_lua_config.h>
#include "window_ide.h"

using namespace System;
using namespace Windows::Forms;
using namespace System::Collections;
using namespace System::Collections::Generic;


namespace xray {
namespace editor {

input_engine::input_engine( editor_world& world ) : 
m_world					( world ),
m_active_combination	( nullptr ),
m_waiting_for_second	( false ),
m_first_keys_released	( true ),
m_paused				( false ),
m_keys_count			( 0 ),
m_active_list			( nullptr ),
m_active_action_index	( -1 )
{
	m_key_converter		= gcnew KeysConverter ();
}

input_engine::~input_engine	( )
{
	this->!input_engine	( );
}

input_engine::!input_engine	( )
{
	m_keys_to_items.Clear();

	IDictionaryEnumerator^ enm = m_names_to_items.GetEnumerator();

	while( enm->MoveNext() )
	{
		Generic::List<action_item^>^ list = (Generic::List<action_item^>^)enm->Value;

		ASSERT( list->Count > 0 );
		for each ( action_item^ act in list )
			delete act->m_action;
	}
}

void input_engine::register_action	( xray::editor::input_action^  action )
{
	List<action_item^>^ items = (List<action_item^>^)m_names_to_items[action->name()];

	ASSERT( items == nullptr, "An action with the specified name already registerd!"  );
	if( items != nullptr )
		return;

	action_item^ new_item = gcnew action_item( action, nullptr, nullptr );

	List<action_item^>^ list = gcnew List<action_item^>( );
	list->Add( new_item );
	m_names_to_items[action->name()] = list;
}

void input_engine::register_action	( xray::editor::input_action^  action, String^ combination )
{
	register_action	( action );
	add_action_shortcut( action->name(), combination );
}

bool input_engine::unregister_action	( System::String^ name, bool destroy )
{
	List<action_item^>^ items = (List<action_item^>^)m_names_to_items[name];
	if( items == nullptr )
		return false;

	ASSERT( items->Count > 0 );
	xray::editor::input_action^ action = items[0]->m_action;

	List<action_item^> copied = items;

	for( int i = copied.Count-1; i >= 0 ; --i )
		remove_action_shortcut( name, copied[i]->m_first_keys + ", " + copied[i]->m_second_keys );

	m_names_to_items.Remove( name );

	if( destroy )
		delete action;

	return true;
}

String^ input_engine::arrange_keys	( String^ keys_string )
{
	array<String^>^	split	= keys_string->Split( '+' );
	ArrayList^		keys	= gcnew ArrayList();

	Collections::IEnumerator^ it = split->GetEnumerator();
	Keys modifiers = Keys::None;

	while ( it->MoveNext() )
	{
		String^ s = safe_cast<String^>(it->Current);
		Keys key;
		if (  !s->Trim()->Equals( "" ) && (key = try_convert( s )) != Keys::None )
		{
			if( (key&Keys::Modifiers) != Keys::None )
			{
				modifiers = modifiers|(key&Keys::Modifiers);
			}
			else if( (key&Keys::KeyCode) != Keys::None && !keys->Contains( key&Keys::KeyCode ) )
				keys->Add( key&Keys::KeyCode );
		}
		else
			return nullptr;
	}

	return combine_string( modifiers,  keys );
}

Keys input_engine::try_convert		( String^ str )
{
	Keys key = Keys::None;
	try
	{
		key = *((Keys^)m_key_converter->ConvertFromString( str ));
	}
	catch(...)
	{
		return Keys::None;
	}

	return key;
}

String^	input_engine::combine_string	( Keys modifiers, ArrayList^ keys )
{
	String^ result = gcnew String("");
	keys->Sort();
	
	if( keys->Count > 0)
		result = m_key_converter->ConvertToString( modifiers|((Keys)keys[0]) );
	else
		result = m_key_converter->ConvertToString( modifiers );

	for ( int i = 1; i < keys->Count; ++i  )
	{
		result += "+" + m_key_converter->ConvertToString( keys[i] );
	}

	return result;
}

//void input_engine::extract_modifyer_keys( System::Collections::ArrayList^ keys, System::Windows::Forms::Keys %modifiers )
//{
//	int ind = keys->IndexOf( Keys::ControlKey );
//	if( ind != -1 ) 
//	{
//		modifiers = modifiers|Keys::Control;
//		keys->RemoveAt( ind );
//	}
//
//	ind = keys->IndexOf( Keys::ShiftKey );
//	if( ind != -1 ) 
//	{
//		modifiers = modifiers|Keys::Shift;
//		keys->RemoveAt( ind );
//	}
//
//	ind = keys->IndexOf( Keys::Menu ); //AltKey
//	if( ind != -1 ) 
//	{
//		modifiers = modifiers|Keys::Alt;
//		keys->RemoveAt( ind );
//	}
//}

void input_engine::execute			( )
{
	String^ combination		= "";
	int key_count			= m_world.ide()->get_current_keys_string( combination );
	// If no Action is active.
	if( m_active_list == nullptr )
	{
		if( m_paused )
			return;

		// This condition will guarantee that no action will be started 
		// if just some keys are released from another key combination
		// otherwise any new key is started.
		if( key_count != m_keys_count )
		{
			m_keys_count = key_count;
			return;
		}

		// Save keys count to be able to detect if any new key is pressed.
		m_keys_count = key_count;

		// Check id there is any input action with the current combination.
		if( !m_keys_to_items.ContainsKey( combination ) )
			return;

		// Get actions list with the same first key combination.
		List<action_item^>^ actions_list = (List<action_item^>^)m_keys_to_items[combination];
		ASSERT( actions_list );

		for( int i = 0; i < actions_list->Count; ++i )
		{
			action_item^ action = (action_item^)actions_list[i];

			// If the current has second key combination then set the current actions list as an active and
			// set flags to wait for the second key combination be processed.
			if( action->m_second_keys != nullptr )
			{
				m_waiting_for_second	= true;
				// The index of the action that is first in the actions list that wait for second combination.
				m_active_action_index	= i;
				m_active_combination	= combination;
				m_active_list			= actions_list;
				m_first_keys_released	= false;
				break;
			}
			// If the action hasn't second key combination then just try to start it.
			// otherwise consider the next one.
			else if( action->m_action->enabled() && action->m_action->capture() )
			{
//				action_item->m_action->start();
				m_active_list 			= actions_list;
				m_active_action_index	= i;
				m_active_combination	= combination;
				break;
			}
		}
	}
	else
	{
		// If we don't wait for a second key combination
		if( !m_waiting_for_second )
		{
			// Check if the current key combination still matches to the active actions.
			// If no just release the input.
			if( m_active_combination != combination ) 
			{
				((action_item^)m_active_list[m_active_action_index])->m_action->release();
				m_world.get_picker().clear_cache();//on_input_action_finished	();
				m_active_list						= nullptr;
				m_active_action_index				= -1;
			}
			else 
				// If the combination still matches to the active action then just execute it.
				((action_item^)m_active_list[m_active_action_index])->m_action->execute();
		}
		else
		{
			int local_ind = 0;
			// Consider all actions starting from m_active_action_index index in the current actions list 
			// that are waiting for their second combination.
			if( key_count > m_keys_count && m_first_keys_released )
			while(	m_active_action_index+local_ind < m_active_list->Count )
			{
				ASSERT( ((action_item^)m_active_list[m_active_action_index+local_ind])->m_second_keys != nullptr );

				// If any action is matches to the key combination then try to start it.
				action_item^ itm = (action_item^)m_active_list[m_active_action_index+local_ind];

				if( combination == itm->m_second_keys && itm->m_action->enabled() && itm->m_action->capture() )
				{
					m_active_combination	= combination;
					m_active_action_index	= m_active_action_index + local_ind;
					m_waiting_for_second	= false;
					
					break;
				}
				++local_ind;
			}

			if( m_waiting_for_second )
			{
				// If no corresponding action is found and any key is released
				// and if whole keys in the first combination are released that means that 
				// we are done also with the second combination and releasing input.
				if( (m_first_keys_released && key_count < m_keys_count)
					|| (!m_first_keys_released && key_count > m_keys_count) )
				{
					m_waiting_for_second	= false;
					m_active_combination	= nullptr;
					m_active_list			= nullptr;
					m_active_action_index	= -1;
				}
				else
				{
					// m_keys_count == 0 then in any way the first combination keys all are released.
					if( m_keys_count == 0 )
						m_first_keys_released = true;

					// Save the current keys count to able to figure out 
					//if any new key is pressed or one is released.
					m_keys_count			= key_count;
				}
			}

		}
	}
}

ArrayList^ input_engine::get_public_actions	( )
{
 	ArrayList^ actions = gcnew ArrayList();
// 
// 	IDictionaryEnumerator^ iter = m_keys_to_items.GetEnumerator();
// 	while ( iter->MoveNext() )
// 	{
// 		ArrayList^ actions_list = (ArrayList^)iter->Value;
// 		ASSERT( actions_list != nullptr );
// 
// 		for( int i = 0; i < actions_list->Count; ++i )
// 		{
// 			input_action^ action_item = (input_action^)actions_list[i];
// 			ASSERT( action_item != nullptr );
// 			if( action_item->is_public() )
// 				actions->Add( (input_action^)action_item );
// 		}
// 	}
	return actions;
}

List<String^>^ input_engine::get_action_keys_list	( System::String^ name )
{
	List<action_item^>^ items = (List<action_item^>^)m_names_to_items[name];
	if( items == nullptr || items[0]->m_first_keys == nullptr  )
	{
		return gcnew List<String^>( );
	}

	
	List<String^>^ result = gcnew List<String^>( items->Count );

	for( int i = 0; i< items->Count; ++i )
	{
// 		String^ str = items[i]->m_first_keys;
// 		if( items[i]->m_second_keys != nullptr )
// 			str += ", " + items[i]->m_second_keys;

		result->Add( items[i]->joined_keys() );
	}

	return result;
}

bool input_engine::remove_action_shortcut	(	System::String^ name, System::String^ combination )
{

	array<String^>^ keys = combination->Split(',');
	if( keys->Length == 0 || keys->Length > 2)
	{
		ASSERT( false, "Incorrect keys combination." );
		return false;
	}

	List<action_item^>^ items = (List<action_item^>^)m_names_to_items[name];

	if( items == nullptr )
		return false;

	ASSERT( items->Count != 0 );

	keys[0] = keys[0]->Trim();
	if( keys->Length == 2 )
		keys[1] = keys[1]->Trim();

	if( keys[0]->Length == 0 ) 
		return false;

	ASSERT( m_keys_to_items.ContainsKey(keys[0]) );

	for( int i = 0; i< items->Count; ++i )
	{
		if( (items[i]->m_first_keys == keys[0]) && ( (keys->Length == 2 && items[i]->m_second_keys == keys[1]) 
												|| (keys->Length == 1 && items[i]->m_second_keys == nullptr ) ) )
		{
			List<action_item^>^ keys_items = (List<action_item^>^)m_keys_to_items[ keys[0] ];
			keys_items->Remove( items[i] );

			if( items->Count == 1 )
			{
				items[0]->m_first_keys = nullptr;
				items[0]->m_second_keys = nullptr;
			}
			else
				items->RemoveAt(i);

			break;
		}
	}

	return true;
}


bool input_engine::add_action_shortcut(	System::String^ name, System::String^ combination )
{
	List<action_item^>^ items = (List<action_item^>^)m_names_to_items[name];
	if( items == nullptr )
		return false;

	ASSERT( items->Count >0 );

	xray::editor::input_action^ action = items[0]->m_action;

	String^ keys_first;
	String^ keys_second;
	excract_shortcut_pair	( combination, keys_first, keys_second );
	
	ASSERT( keys_first != nullptr );

	action_item^ new_item = gcnew action_item( action, keys_first, keys_second );

	List<action_item^>^ list;
	// If already there are any actions with the same first key combination 
	// than adding to the existing list 
	//
	if( m_keys_to_items.ContainsKey( keys_first ) ) 
	{

		// A new action is added at the end of the list of already registered actions 
		// with the same first key combination, but the ones which haven't second key combination 
		// have higher priority, so those are added at the end of the last action with no second key combination.
		//
		list = (List<action_item^>^)m_keys_to_items[keys_first];

		for( int i = 0; i< list->Count; ++i)
		{
			if( new_item->m_action->name() == list[i]->m_action->name() 
				&&	new_item->m_second_keys == list[i]->m_second_keys )
			{
				ASSERT( false, "An action with the specified name and kays is allready registered!" );
				return false;
			}
		}

		int ind = 0;

		// Find the last action in the list with no second combination.
		if( keys_second == nullptr )
		{
			while( ind < list->Count && ((action_item^)list[ind])->m_second_keys == nullptr )
				++ind;
		}

		// Insert the action at the specified index.
		list->Insert( ind, new_item );
	}
	else
	{
		// If there isn't any action with the same first key combination than
		// just adding a new list of actions with the new action in it.
		//
		list = gcnew List<action_item^>( );
		list->Insert( 0, new_item );
		m_keys_to_items[keys_first] = list;
	}


	if( m_names_to_items.ContainsKey( action->name() ) ) 
	{
		list = (List<action_item^>^)m_names_to_items[action->name()];
		if( list[0]->m_first_keys == nullptr )
			list[0] = new_item;
		else
			list->Insert( 0, new_item );
	}
	else
	{
		list = gcnew List<action_item^>( );
		list->Add( new_item );
		m_names_to_items[action->name()] = list;
	}
	return true;
}

void input_engine::stop			( )
{
	ASSERT( !m_paused, "The engine is already stopped." );

	m_paused = true;
	if( m_active_list )
	{
		((action_item^)m_active_list[m_active_action_index])->m_action->release();
		m_world.get_picker().clear_cache(); //on_input_action_finished	();
		m_active_list						= nullptr;
		m_active_action_index				= -1;
	}
	m_waiting_for_second					= false;
	m_active_combination					= nullptr;
}

void input_engine::resume		( )
{
	ASSERT( m_paused, "The engine is not stopped." );
	m_paused = false;
}

void input_engine::excract_shortcut_pair	( String^ combination, String^& keys_first, String^& keys_second )
{
	array<String^>^ keys = combination->Split(',');
	keys_first = nullptr;
	if( keys->Length == 0 || keys->Length > 2)
	{
		ASSERT( false, "Incorrect keys combination." );
		return;
	}

	// Arrange keys in a correct order and case.
	keys_first	= arrange_keys( keys[0] );

	// The same with the second key combination
	keys_second = nullptr;
	if(  keys->Length == 2 )
	{
		ASSERT( keys[1] != "" );
		keys_second = arrange_keys( keys[1] );
		ASSERT( keys_second != nullptr && keys_second != "" );
	}
}

bool input_engine::get_actions_by_shortcut( System::String^ combination, Generic::List<String^>^& actions, Generic::List<String^>^& shortcuts )
{
	String^ keys_first;
	String^ keys_second;
	actions = gcnew List<String^>();
	shortcuts = gcnew List<String^>();

	excract_shortcut_pair	( combination, keys_first, keys_second );

	if( keys_first == nullptr || keys_first == "" )
	{
		ASSERT( false, "Incorrect keys combination!" );
		return false;
	}

	List<action_item^>^ list;

	list = (List<action_item^>^)m_keys_to_items[keys_first];
	if( list != nullptr )
		for ( int i = 0; i < list->Count; i++ )
		{
			actions->Add ( list[i]->m_action->name() );
			String^ shortcut_str = list[i]->m_first_keys;
			if( list[i]->m_second_keys != nullptr ) 
				shortcut_str += "" + list[i]->m_second_keys;

			shortcuts->Add ( shortcut_str );
		}

	return actions->Count > 0;
}

bool input_engine::get_actions_by_shortcut( System::String^ combination, Generic::List<String^>^& actions )
{
	String^ keys_first;
	String^ keys_second;
	actions = gcnew List<String^>();

	excract_shortcut_pair	( combination, keys_first, keys_second );

	if( keys_first != nullptr && keys_first != "" )
	{
		ASSERT( false, "Incorrect keys combination!" );
		return false;
	}

	List<action_item^>^ list;

	list = (List<action_item^>^)m_keys_to_items[keys_first];
	for ( int i = 0; i < list->Count; i++ )
	{
		if( list[i]->m_second_keys == keys_second )
			actions->Add ( list[i]->m_action->name() );
	}

	return actions->Count > 0;
}

void input_engine::remove_all_shortcuts	( )
{
	m_keys_to_items.Clear();

	IDictionaryEnumerator^ enm = m_names_to_items.GetEnumerator();

	while( enm->MoveNext() )
	{
		Generic::List<action_item^>^ list = (Generic::List<action_item^>^)enm->Value;

		ASSERT( list->Count > 0 );
		list->RemoveRange( 1, list->Count-1 );

		list[0]->m_first_keys = nullptr;
		list[0]->m_second_keys = nullptr;
	}
}

void input_engine::load ( )
{
	RegistryKey^ product_key		= m_world.ide()->base_registry_key(); 
	RegistryKey^ keybindings_key	= get_sub_key(product_key, "keybindings");

	array<String^>^	value_names		= keybindings_key->GetValueNames();
	
	if(value_names->GetLength(0)>0)
		remove_all_shortcuts		();

	for each(System::String^ name in value_names)
	{
		String^ value				= dynamic_cast<String^>(keybindings_key->GetValue(name));

		array<String^>^ str_pair	= value->Split('-');
		ASSERT( str_pair->Length == 2 );
		add_action_shortcut( str_pair[0]->Trim(), str_pair[1]->Trim() );
	}

	keybindings_key->Close			();
	product_key->Close				();
}

void input_engine::save ( )
{
	RegistryKey^ product_key		= m_world.ide()->base_registry_key(); 
	RegistryKey^ keybindings_key	= get_sub_key(product_key, "keybindings");

	Collections::Hashtable^	curr_keys_to_items	= %m_keys_to_items;
	Collections::Hashtable^	next_keys_to_items	= nullptr;

	// The save procedure is a little complicated that it manages right sequence of 
	// saved actions to keep right priority of actions_items. There is two kind of priorities:
	// Priority in the list of action with the same first shortcut and priority between actions_items
	// that execute the same action.
	u32 index = 0;
	do 
	{
		// Storage to save postponed records that need to wait others to be saved before.
		next_keys_to_items = gcnew Collections::Hashtable();
		Collections::IEnumerator^ iter = curr_keys_to_items->Keys->GetEnumerator();
		while( iter->MoveNext() )
		{
			String^ curr_key = ((String^)iter->Current);
			Generic::List<action_item^>^ actions = (Generic::List<action_item^>^)curr_keys_to_items[curr_key];

			for( int i = 0;  i < actions->Count; ++i )
			{
				Generic::List<action_item^>^ action_by_name = (Generic::List<action_item^>^)m_names_to_items[actions[i]->m_action->name()];
				bool breaked = false;

				if( action_by_name != nullptr )
					for( int j = 0; action_by_name[j] != actions[i] && j < action_by_name->Count; ++j )
					{
						Generic::List<action_item^>^ backtrack_list = (Generic::List<action_item^>^)curr_keys_to_items[action_by_name[j]->m_first_keys];
						if( backtrack_list != nullptr && backtrack_list->Contains(action_by_name[j]))
						{
							next_keys_to_items[curr_key] = actions->GetRange( 0, i+1 );
							breaked = true;
							break;
						}
					}

				if( breaked )
					break;
				
				String^ binding_str			= actions[i]->m_action->name() + "-" + actions[i]->joined_keys();
				keybindings_key->SetValue(index.ToString("D4"), binding_str); 
				++index;
			}
		}
		curr_keys_to_items = next_keys_to_items;
	} 
	while( next_keys_to_items->Count > 0 );
	
	keybindings_key->Close();
	product_key->Close();
}

} // namespace editor
} // namespace xray
