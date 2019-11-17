////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#pragma once


using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

public delegate bool boolean_getter();
#include "editor_ide.h"

namespace xray {
namespace editor {
	ref class window_view;
	ref class library_dockable;
	ref class options_dialog;
	ref class action_engine;
	ref class gui_binder;

	class property_holder;

	#pragma unmanaged
		class editor_world;
	#pragma managed

	/// <summary>
	/// Summary for window_ide
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class window_ide : public System::Windows::Forms::Form, public editor_ide
	{
		typedef Form	super;
	public:
		window_ide(	xray::editor::editor_world& world ) :
			m_editor_world			( world )
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			in_constructor	( );
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~window_ide()
		{
			if (components)
			{
				delete components;
			}

			custom_finalize	( );
		}

	protected: 

	protected: 

	private: System::Windows::Forms::MenuStrip^  menuStrip;
	private: System::Windows::Forms::ToolStripMenuItem^  ViewMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  HelpMenuItem;

	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog;
	private: System::Windows::Forms::SaveFileDialog^  saveFileDialog;



	private: System::Windows::Forms::ToolStripMenuItem^  outputToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  ToolsMenuItem;


	private: System::Windows::Forms::ToolStripMenuItem^  optionsToolStripMenuItem;


	private: System::Windows::Forms::ToolStripMenuItem^  FileMenuItem;

	private: System::Windows::Forms::ToolStripSeparator^  toolStripMenuItem1;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  EditMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  recentFilesToolStripMenuItem;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel0;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel2;
	private: WeifenLuo::WinFormsUI::Docking::DockPanel^  Editor;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel3;









	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(window_ide::typeid));
			WeifenLuo::WinFormsUI::Docking::DockPanelSkin^  dockPanelSkin1 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanelSkin());
			WeifenLuo::WinFormsUI::Docking::AutoHideStripSkin^  autoHideStripSkin1 = (gcnew WeifenLuo::WinFormsUI::Docking::AutoHideStripSkin());
			WeifenLuo::WinFormsUI::Docking::DockPanelGradient^  dockPanelGradient1 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanelGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient1 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::DockPaneStripSkin^  dockPaneStripSkin1 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPaneStripSkin());
			WeifenLuo::WinFormsUI::Docking::DockPaneStripGradient^  dockPaneStripGradient1 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPaneStripGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient2 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::DockPanelGradient^  dockPanelGradient2 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanelGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient3 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::DockPaneStripToolWindowGradient^  dockPaneStripToolWindowGradient1 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPaneStripToolWindowGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient4 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient5 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::DockPanelGradient^  dockPanelGradient3 = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanelGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient6 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			WeifenLuo::WinFormsUI::Docking::TabGradient^  tabGradient7 = (gcnew WeifenLuo::WinFormsUI::Docking::TabGradient());
			this->menuStrip = (gcnew System::Windows::Forms::MenuStrip());
			this->FileMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->recentFilesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->EditMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ViewMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->outputToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->optionsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->HelpMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->openFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
			this->saveFileDialog = (gcnew System::Windows::Forms::SaveFileDialog());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel0 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabel2 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->Editor = (gcnew WeifenLuo::WinFormsUI::Docking::DockPanel());
			this->toolStripStatusLabel3 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->menuStrip->SuspendLayout();
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// menuStrip
			// 
			this->menuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {this->FileMenuItem, this->EditMenuItem, 
				this->ViewMenuItem, this->ToolsMenuItem, this->HelpMenuItem});
			resources->ApplyResources(this->menuStrip, L"menuStrip");
			this->menuStrip->Name = L"menuStrip";
			// 
			// FileMenuItem
			// 
			this->FileMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->recentFilesToolStripMenuItem, 
				this->toolStripMenuItem1, this->exitToolStripMenuItem});
			this->FileMenuItem->Name = L"FileMenuItem";
			resources->ApplyResources(this->FileMenuItem, L"FileMenuItem");
			this->FileMenuItem->Tag = L"0";
			this->FileMenuItem->DropDownOpening += gcnew System::EventHandler(this, &window_ide::FileMenuItem_DropDownOpening);
			// 
			// recentFilesToolStripMenuItem
			// 
			this->recentFilesToolStripMenuItem->Name = L"recentFilesToolStripMenuItem";
			resources->ApplyResources(this->recentFilesToolStripMenuItem, L"recentFilesToolStripMenuItem");
			// 
			// toolStripMenuItem1
			// 
			this->toolStripMenuItem1->Name = L"toolStripMenuItem1";
			resources->ApplyResources(this->toolStripMenuItem1, L"toolStripMenuItem1");
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			resources->ApplyResources(this->exitToolStripMenuItem, L"exitToolStripMenuItem");
			this->exitToolStripMenuItem->Tag = L"10";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &window_ide::exitToolStripMenuItem_Click);
			// 
			// EditMenuItem
			// 
			this->EditMenuItem->Name = L"EditMenuItem";
			resources->ApplyResources(this->EditMenuItem, L"EditMenuItem");
			this->EditMenuItem->Tag = L"10";
			// 
			// ViewMenuItem
			// 
			this->ViewMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->outputToolStripMenuItem});
			this->ViewMenuItem->Name = L"ViewMenuItem";
			resources->ApplyResources(this->ViewMenuItem, L"ViewMenuItem");
			this->ViewMenuItem->Tag = L"20";
			// 
			// outputToolStripMenuItem
			// 
			resources->ApplyResources(this->outputToolStripMenuItem, L"outputToolStripMenuItem");
			this->outputToolStripMenuItem->Name = L"outputToolStripMenuItem";
			this->outputToolStripMenuItem->Tag = L"10";
			// 
			// ToolsMenuItem
			// 
			this->ToolsMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->optionsToolStripMenuItem});
			this->ToolsMenuItem->Name = L"ToolsMenuItem";
			resources->ApplyResources(this->ToolsMenuItem, L"ToolsMenuItem");
			this->ToolsMenuItem->Tag = L"30";
			// 
			// optionsToolStripMenuItem
			// 
			this->optionsToolStripMenuItem->Name = L"optionsToolStripMenuItem";
			resources->ApplyResources(this->optionsToolStripMenuItem, L"optionsToolStripMenuItem");
			this->optionsToolStripMenuItem->Tag = L"10";
			this->optionsToolStripMenuItem->Click += gcnew System::EventHandler(this, &window_ide::optionsToolStripMenuItem_Click);
			// 
			// HelpMenuItem
			// 
			this->HelpMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->aboutToolStripMenuItem});
			this->HelpMenuItem->Name = L"HelpMenuItem";
			resources->ApplyResources(this->HelpMenuItem, L"HelpMenuItem");
			this->HelpMenuItem->Tag = L"40";
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			resources->ApplyResources(this->aboutToolStripMenuItem, L"aboutToolStripMenuItem");
			this->aboutToolStripMenuItem->Tag = L"10";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &window_ide::aboutToolStripMenuItem_Click);
			// 
			// openFileDialog
			// 
			this->openFileDialog->FileName = L"openFileDialog";
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->toolStripStatusLabel0, 
				this->toolStripStatusLabel1, this->toolStripStatusLabel2, this->toolStripStatusLabel3});
			resources->ApplyResources(this->statusStrip1, L"statusStrip1");
			this->statusStrip1->Name = L"statusStrip1";
			// 
			// toolStripStatusLabel0
			// 
			this->toolStripStatusLabel0->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripStatusLabel0->Name = L"toolStripStatusLabel0";
			resources->ApplyResources(this->toolStripStatusLabel0, L"toolStripStatusLabel0");
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			resources->ApplyResources(this->toolStripStatusLabel1, L"toolStripStatusLabel1");
			// 
			// toolStripStatusLabel2
			// 
			this->toolStripStatusLabel2->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripStatusLabel2->Name = L"toolStripStatusLabel2";
			resources->ApplyResources(this->toolStripStatusLabel2, L"toolStripStatusLabel2");
			// 
			// Editor
			// 
			this->Editor->ActiveAutoHideContent = nullptr;
			resources->ApplyResources(this->Editor, L"Editor");
			this->Editor->DockBackColor = System::Drawing::SystemColors::Control;
			this->Editor->DocumentStyle = WeifenLuo::WinFormsUI::Docking::DocumentStyle::DockingSdi;
			this->Editor->Name = L"Editor";
			dockPanelGradient1->EndColor = System::Drawing::SystemColors::ControlLight;
			dockPanelGradient1->StartColor = System::Drawing::SystemColors::ControlLight;
			autoHideStripSkin1->DockStripGradient = dockPanelGradient1;
			tabGradient1->EndColor = System::Drawing::SystemColors::Control;
			tabGradient1->StartColor = System::Drawing::SystemColors::Control;
			tabGradient1->TextColor = System::Drawing::SystemColors::ControlDarkDark;
			autoHideStripSkin1->TabGradient = tabGradient1;
			dockPanelSkin1->AutoHideStripSkin = autoHideStripSkin1;
			tabGradient2->EndColor = System::Drawing::SystemColors::ControlLightLight;
			tabGradient2->StartColor = System::Drawing::SystemColors::ControlLightLight;
			tabGradient2->TextColor = System::Drawing::SystemColors::ControlText;
			dockPaneStripGradient1->ActiveTabGradient = tabGradient2;
			dockPanelGradient2->EndColor = System::Drawing::SystemColors::Control;
			dockPanelGradient2->StartColor = System::Drawing::SystemColors::Control;
			dockPaneStripGradient1->DockStripGradient = dockPanelGradient2;
			tabGradient3->EndColor = System::Drawing::SystemColors::ControlLight;
			tabGradient3->StartColor = System::Drawing::SystemColors::ControlLight;
			tabGradient3->TextColor = System::Drawing::SystemColors::ControlText;
			dockPaneStripGradient1->InactiveTabGradient = tabGradient3;
			dockPaneStripSkin1->DocumentGradient = dockPaneStripGradient1;
			tabGradient4->EndColor = System::Drawing::SystemColors::ActiveCaption;
			tabGradient4->LinearGradientMode = System::Drawing::Drawing2D::LinearGradientMode::Vertical;
			tabGradient4->StartColor = System::Drawing::SystemColors::GradientActiveCaption;
			tabGradient4->TextColor = System::Drawing::SystemColors::ActiveCaptionText;
			dockPaneStripToolWindowGradient1->ActiveCaptionGradient = tabGradient4;
			tabGradient5->EndColor = System::Drawing::SystemColors::Control;
			tabGradient5->StartColor = System::Drawing::SystemColors::Control;
			tabGradient5->TextColor = System::Drawing::SystemColors::ControlText;
			dockPaneStripToolWindowGradient1->ActiveTabGradient = tabGradient5;
			dockPanelGradient3->EndColor = System::Drawing::SystemColors::ControlLight;
			dockPanelGradient3->StartColor = System::Drawing::SystemColors::ControlLight;
			dockPaneStripToolWindowGradient1->DockStripGradient = dockPanelGradient3;
			tabGradient6->EndColor = System::Drawing::SystemColors::GradientInactiveCaption;
			tabGradient6->LinearGradientMode = System::Drawing::Drawing2D::LinearGradientMode::Vertical;
			tabGradient6->StartColor = System::Drawing::SystemColors::GradientInactiveCaption;
			tabGradient6->TextColor = System::Drawing::SystemColors::ControlText;
			dockPaneStripToolWindowGradient1->InactiveCaptionGradient = tabGradient6;
			tabGradient7->EndColor = System::Drawing::Color::Transparent;
			tabGradient7->StartColor = System::Drawing::Color::Transparent;
			tabGradient7->TextColor = System::Drawing::SystemColors::ControlDarkDark;
			dockPaneStripToolWindowGradient1->InactiveTabGradient = tabGradient7;
			dockPaneStripSkin1->ToolWindowGradient = dockPaneStripToolWindowGradient1;
			dockPanelSkin1->DockPaneStripSkin = dockPaneStripSkin1;
			this->Editor->Skin = dockPanelSkin1;
			// 
			// toolStripStatusLabel3
			// 
			this->toolStripStatusLabel3->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripStatusLabel3->Name = L"toolStripStatusLabel3";
			resources->ApplyResources(this->toolStripStatusLabel3, L"toolStripStatusLabel3");
			// 
			// window_ide
			// 
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->Editor);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->menuStrip);
			this->KeyPreview = true;
			this->MainMenuStrip = this->menuStrip;
			this->Name = L"window_ide";
			this->SizeChanged += gcnew System::EventHandler(this, &window_ide::window_ide_SizeChanged);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &window_ide::window_ide_KeyUp);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &window_ide::window_ide_FormClosing);
			this->LocationChanged += gcnew System::EventHandler(this, &window_ide::window_ide_LocationChanged);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &window_ide::window_ide_KeyDown);
			this->menuStrip->ResumeLayout(false);
			this->menuStrip->PerformLayout();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private:
			void	in_constructor		( );
			void	custom_finalize		( );
			void	on_idle				( System::Object ^sender, System::EventArgs ^event_args );
	WeifenLuo::WinFormsUI::Docking::IDockContent ^reload_content	(System::String ^persist_string);

public:
	inline	bool	in_idle				( ) { return !!m_in_idle; }
	inline	editor_world&	get_editor_world( ) { return m_editor_world; }
	inline	window_view%	view		( ) { return *m_view; }
			void	add_tab				( System::String^ tab_id, xray::editor::property_holder& property_holder );
			void	add_window			( WeifenLuo::WinFormsUI::Docking::DockContent^ form, WeifenLuo::WinFormsUI::Docking::DockState dock );


			Point	get_mouse_pos_infinte( );
			void	window_ide_KeyDown	(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
			void	window_ide_KeyUp	(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);


	inline	WeifenLuo::WinFormsUI::Docking::DockPanel^ get_editor( )			{ return Editor; }


private:
	xray::editor::editor_world&		m_editor_world;
	window_view^					m_view;
	System::Drawing::Rectangle^		m_window_rectangle;
	u32								m_in_idle;
	library_dockable^				m_library_dock;
	
	System::Collections::Hashtable	m_images;

	action_engine^					m_gui_action_angine;	
	options_dialog^					m_options_dialog;
private:
	System::Void	window_ide_FormClosing		(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
	System::Void	window_ide_LocationChanged	(System::Object^  sender, System::EventArgs^  e);
	System::Void	window_ide_SizeChanged		(System::Object^  sender, System::EventArgs^  e);
	System::Void	exitToolStripMenuItem_Click	(System::Object^  sender, System::EventArgs^  e);
	
	System::Void	aboutToolStripMenuItem_Click		(System::Object^  sender, System::EventArgs^  e);
	System::Void	optionsToolStripMenuItem_Click		(System::Object^  sender, System::EventArgs^  e);

	ToolStripMenuItem^ get_parent_menu_item				(System::String^ key);

public:
			bool			is_mouse_in_view			();

	virtual RegistryKey^	base_registry_key			();
	virtual void			register_image				(System::String^ key, xray::editor_base::images16x16 image_id );
	virtual System::Drawing::Image^ get_image			(System::String^ key);
	virtual void			add_menu_item				(System::String^ key, System::String^ text, int prio);
	virtual void			add_menu_item				(gui_binder^ binder, System::String^ action_name, System::String^ key, int prio);
	virtual void			add_button_item				(System::String^ key, int prio);
	virtual void			add_button_item				(gui_binder^ binder, System::String^ action_name, System::String^ key, int prio);
	virtual void			show_tab					(WeifenLuo::WinFormsUI::Docking::DockContent^ tab);
	virtual void			show_tab					(WeifenLuo::WinFormsUI::Docking::DockContent^ tab, WeifenLuo::WinFormsUI::Docking::DockState state);

	virtual ArrayList^		get_held_keys				();
	virtual int				get_current_keys_string		( System::String^& combination );

	virtual void			get_mouse_position			(xray::math::int2& );
	virtual void			get_mouse_position			(System::Drawing::Point%);
	virtual void			set_mouse_position			(System::Drawing::Point);
	virtual void			get_view_size				(xray::math::int2&);
	virtual void			get_view_size				(System::Drawing::Size%);
	virtual void			set_status_label			(int idx, System::String^ text);
	virtual void			set_mouse_sensivity			(float sensivity);
	virtual IWin32Window^	wnd							() {return this;}
	virtual options_manager^ get_options_manager		();
	
			void			load_contents_layout		();
	private: System::Void	FileMenuItem_DropDownOpening(System::Object^  sender, System::EventArgs^  e);
	private: void			RecentFileMenuItem_Click	(System::Object^ sender, System::EventArgs^ e);

public:
	inline	ImageList^		images16x16					( ) { return m_images16x16; }
	virtual void			WndProc						(Message% m) override;

private:
	void					key_combination_changed		( System::String^ combination );
	void					OnWM_Activate				(WPARAM wParam, LPARAM lParam);
	ImageList^				m_images16x16;
};

} // namespace editor
} // namespace xray
