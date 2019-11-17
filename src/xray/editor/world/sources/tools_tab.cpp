#include "pch.h"
#include "tools_tab.h"

using xray::editor::tools_tab;

void tools_tab::add_tab(pcstr name, System::Windows::Forms::UserControl^ tab)
{
	tab_control->SuspendLayout();

	System::Windows::Forms::TabPage^ page = (gcnew System::Windows::Forms::TabPage());
	page->Text	= gcnew System::String(name);
//	page->Controls->SuspendLayout();
	page->Controls->Add(tab);
	tab->Dock = System::Windows::Forms::DockStyle::Fill;
	tab_control->Controls->Add(page);
//	page->Controls->ResumeLayout();
	tab_control->ResumeLayout();
}

int tools_tab::selected_tab_index	()
{
	return tab_control->SelectedIndex;
}