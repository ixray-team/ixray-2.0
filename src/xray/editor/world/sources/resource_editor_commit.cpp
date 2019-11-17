////////////////////////////////////////////////////////////////////////////
//	Created		: 18.03.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource_editor_commit.h"


namespace xray{
namespace editor{

void		resource_editor_commit::m_cancel_button_Click			(Object^ , EventArgs^ )
{
	this->Close();
}

void		resource_editor_commit::revertChanges_Click				(Object^ , EventArgs^ )
{
	m_list_box->SuspendLayout();
	for (int i = m_list_box->SelectedItems->Count-1; i >= 0; i--)
	{
		String^ item = m_list_box->SelectedItems[i]->ToString();
		
		m_changed_options->Remove(item);
		m_list_box->Items->Remove(item);
	}
	m_list_box->ResumeLayout(false);
}

void		resource_editor_commit::m_list_box_MouseUp				(Object^ , MouseEventArgs^ e)
{
	if(e->Button == System::Windows::Forms::MouseButtons::Right)
	{
		if(!m_list_box->GetSelected(m_list_box->IndexFromPoint(e->Location)))
		{
			m_list_box->SelectedItems->Clear();		
			m_list_box->SetSelected		(m_list_box->IndexFromPoint(e->Location), true);
		}
		m_options_context_menu->Show	(m_list_box->PointToScreen(e->Location));
	}
}

void		resource_editor_commit::m_revert_selection_button_Click	(Object^ , EventArgs^ )
{
	m_list_box->SuspendLayout();
	for (int i = m_list_box->SelectedItems->Count-1; i >= 0; i--)
	{
		String^ item = m_list_box->SelectedItems[i]->ToString();
		
		m_changed_options->Remove(item);
		m_list_box->Items->Remove(item);
	}
	m_list_box->ResumeLayout(false);
}

} // namespace editor
} // namespace xray