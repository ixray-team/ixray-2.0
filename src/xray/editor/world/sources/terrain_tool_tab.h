#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "tool_tab.h"
#include <xray/sound/sound.h>

namespace xray {
namespace editor {
ref class tool_terrain;
ref class terrain_heightmap_control_tab;
ref class terrain_core_form;
ref class editor_control_base;
	/// <summary>
	/// Summary for light_tool_tab
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class terrain_tool_tab :	public System::Windows::Forms::UserControl, 
									public tool_tab
	{
	public:
		terrain_tool_tab(tool_terrain^ t):m_tool(t)
		{
			InitializeComponent();
			in_constructor();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~terrain_tool_tab()
		{
			in_destructor();
			if (components)
			{
				delete components;
			}
		}
	void		in_constructor();
	void		in_destructor();
	private: tool_terrain^	m_tool;
			 terrain_core_form^			m_terrain_core_form;
			terrain_heightmap_control_tab^	m_terrain_heightmap_control_tab;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	protected: 




















	private: System::Windows::Forms::GroupBox^  groupBox1;



	private: System::Windows::Forms::ToolStrip^  top_toolStrip;





	private: System::Windows::Forms::Button^  button1;

	private: System::Windows::Forms::PropertyGrid^  active_control_property_grid;
	private: System::Windows::Forms::Splitter^  splitter1;
	private: System::Windows::Forms::Panel^  control_parameters_panel;
	private: System::Windows::Forms::Button^  btn_play;
	private: System::Windows::Forms::Button^  btn_stop;
	private: System::Windows::Forms::ComboBox^  sound_cb;












	private: 

	private: System::ComponentModel::IContainer^  components;
	public: 
	private: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->splitter1 = (gcnew System::Windows::Forms::Splitter());
			this->control_parameters_panel = (gcnew System::Windows::Forms::Panel());
			this->active_control_property_grid = (gcnew System::Windows::Forms::PropertyGrid());
			this->top_toolStrip = (gcnew System::Windows::Forms::ToolStrip());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->btn_play = (gcnew System::Windows::Forms::Button());
			this->btn_stop = (gcnew System::Windows::Forms::Button());
			this->sound_cb = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox1->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->Location = System::Drawing::Point(0, 552);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(239, 22);
			this->statusStrip1->TabIndex = 2;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// groupBox1
			// 
			this->groupBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox1->Controls->Add(this->splitter1);
			this->groupBox1->Controls->Add(this->control_parameters_panel);
			this->groupBox1->Controls->Add(this->active_control_property_grid);
			this->groupBox1->Controls->Add(this->top_toolStrip);
			this->groupBox1->Location = System::Drawing::Point(0, 0);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(239, 494);
			this->groupBox1->TabIndex = 9;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"tools";
			// 
			// splitter1
			// 
			this->splitter1->Dock = System::Windows::Forms::DockStyle::Top;
			this->splitter1->Location = System::Drawing::Point(3, 254);
			this->splitter1->Name = L"splitter1";
			this->splitter1->Size = System::Drawing::Size(233, 3);
			this->splitter1->TabIndex = 12;
			this->splitter1->TabStop = false;
			// 
			// control_parameters_panel
			// 
			this->control_parameters_panel->Dock = System::Windows::Forms::DockStyle::Fill;
			this->control_parameters_panel->Location = System::Drawing::Point(3, 254);
			this->control_parameters_panel->Name = L"control_parameters_panel";
			this->control_parameters_panel->Size = System::Drawing::Size(233, 237);
			this->control_parameters_panel->TabIndex = 11;
			// 
			// active_control_property_grid
			// 
			this->active_control_property_grid->Dock = System::Windows::Forms::DockStyle::Top;
			this->active_control_property_grid->HelpVisible = false;
			this->active_control_property_grid->Location = System::Drawing::Point(3, 41);
			this->active_control_property_grid->Name = L"active_control_property_grid";
			this->active_control_property_grid->Size = System::Drawing::Size(233, 213);
			this->active_control_property_grid->TabIndex = 8;
			this->active_control_property_grid->ToolbarVisible = false;
			// 
			// top_toolStrip
			// 
			this->top_toolStrip->Location = System::Drawing::Point(3, 16);
			this->top_toolStrip->Name = L"top_toolStrip";
			this->top_toolStrip->Size = System::Drawing::Size(233, 25);
			this->top_toolStrip->TabIndex = 6;
			this->top_toolStrip->Text = L"top_toolStrip";
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->button1->Location = System::Drawing::Point(3, 526);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(65, 23);
			this->button1->TabIndex = 14;
			this->button1->Text = L"CORE";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &terrain_tool_tab::button1_Click_1);
			// 
			// btn_play
			// 
			this->btn_play->Location = System::Drawing::Point(101, 497);
			this->btn_play->Name = L"btn_play";
			this->btn_play->Size = System::Drawing::Size(53, 23);
			this->btn_play->TabIndex = 15;
			this->btn_play->Text = L"play";
			this->btn_play->UseVisualStyleBackColor = true;
			this->btn_play->Click += gcnew System::EventHandler(this, &terrain_tool_tab::btn_play_Click);
			// 
			// btn_stop
			// 
			this->btn_stop->Location = System::Drawing::Point(160, 497);
			this->btn_stop->Name = L"btn_stop";
			this->btn_stop->Size = System::Drawing::Size(66, 23);
			this->btn_stop->TabIndex = 16;
			this->btn_stop->Text = L"stop";
			this->btn_stop->UseVisualStyleBackColor = true;
			this->btn_stop->Click += gcnew System::EventHandler(this, &terrain_tool_tab::btn_stop_Click);
			// 
			// sound_cb
			// 
			this->sound_cb->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->sound_cb->FormattingEnabled = true;
			this->sound_cb->Location = System::Drawing::Point(74, 526);
			this->sound_cb->Name = L"sound_cb";
			this->sound_cb->Size = System::Drawing::Size(152, 21);
			this->sound_cb->TabIndex = 17;
			// 
			// terrain_tool_tab
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->sound_cb);
			this->Controls->Add(this->btn_stop);
			this->Controls->Add(this->btn_play);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->statusStrip1);
			this->Name = L"terrain_tool_tab";
			this->Size = System::Drawing::Size(239, 574);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	public:
	virtual System::Windows::Forms::UserControl^	frame () {return this;}
	private: 
	xray::sound::sound_ptr*		m_sound;
	void button1_Click_1(System::Object^  sender, System::EventArgs^  e);
	void on_control_activated(editor_control_base^);
	void on_control_deactivated(editor_control_base^);
	void on_control_property_changed(editor_control_base^);
	void btn_play_Click(System::Object^  sender, System::EventArgs^  e);
	void btn_stop_Click(System::Object^  sender, System::EventArgs^  e);
	void load_sound( pcstr name );
	void on_sound_loaded( resources::queries_result& data );

};
}
}
