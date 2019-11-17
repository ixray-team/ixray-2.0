#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "options_page.h"


namespace xray {
namespace editor {

	class editor_world;

	/// <summary>
	/// Summary for options_page_general
	/// </summary>
	public ref class options_page_general : public System::Windows::Forms::UserControl, public options_page
	{
	public:
		options_page_general	( editor_world& world ):
		  m_editor_world		( world )
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			m_backup_values		= gcnew System::Collections::Hashtable;
		}

		virtual bool	accept_changes		( );
		virtual void	cancel_changes		( );
		virtual void	activate_page		( );
		virtual void	deactivate_page		( );
		virtual bool	changed				( );

		virtual System::Windows::Forms::Control^	get_control	( ) { return this; }

	protected:

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~options_page_general()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::CheckBox^  draw_grid_check_box1;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::CheckBox^  draw_collision_check_box;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::NumericUpDown^  m_near_plane_box;
	private: System::Windows::Forms::NumericUpDown^  m_far_plane_box;
	private: System::Windows::Forms::NumericUpDown^  m_focus_distance_box;
	private: System::Windows::Forms::Label^  label8;





	protected: 

	protected: 

	protected: 

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
			this->draw_grid_check_box1 = (gcnew System::Windows::Forms::CheckBox());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->draw_collision_check_box = (gcnew System::Windows::Forms::CheckBox());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->m_near_plane_box = (gcnew System::Windows::Forms::NumericUpDown());
			this->m_far_plane_box = (gcnew System::Windows::Forms::NumericUpDown());
			this->m_focus_distance_box = (gcnew System::Windows::Forms::NumericUpDown());
			this->label8 = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_near_plane_box))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_far_plane_box))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_focus_distance_box))->BeginInit();
			this->SuspendLayout();
			// 
			// draw_grid_check_box1
			// 
			this->draw_grid_check_box1->AutoSize = true;
			this->draw_grid_check_box1->Location = System::Drawing::Point(108, 18);
			this->draw_grid_check_box1->Name = L"draw_grid_check_box1";
			this->draw_grid_check_box1->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->draw_grid_check_box1->Size = System::Drawing::Size(15, 14);
			this->draw_grid_check_box1->TabIndex = 1;
			this->draw_grid_check_box1->UseVisualStyleBackColor = true;
			this->draw_grid_check_box1->CheckedChanged += gcnew System::EventHandler(this, &options_page_general::draw_grid_check_box1_CheckedChanged);
			// 
			// comboBox1
			// 
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Items->AddRange(gcnew cli::array< System::Object^  >(3) {L"Classic", L"Ruby", L"Winamp"});
			this->comboBox1->Location = System::Drawing::Point(108, 62);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(121, 21);
			this->comboBox1->TabIndex = 2;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(3, 66);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(71, 13);
			this->label1->TabIndex = 3;
			this->label1->Text = L"Color scheme";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(3, 19);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(52, 13);
			this->label2->TabIndex = 4;
			this->label2->Text = L"Draw grid";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(3, 89);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(97, 13);
			this->label3->TabIndex = 5;
			this->label3->Text = L"Background sound";
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(108, 89);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->checkBox1->Size = System::Drawing::Size(15, 14);
			this->checkBox1->TabIndex = 6;
			this->checkBox1->UseVisualStyleBackColor = true;
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(14, 202);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(109, 13);
			this->label4->TabIndex = 7;
			this->label4->Text = L"add new options here";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(3, 43);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(72, 13);
			this->label5->TabIndex = 9;
			this->label5->Text = L"Draw collision";
			// 
			// draw_collision_check_box
			// 
			this->draw_collision_check_box->AutoSize = true;
			this->draw_collision_check_box->Location = System::Drawing::Point(108, 42);
			this->draw_collision_check_box->Name = L"draw_collision_check_box";
			this->draw_collision_check_box->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->draw_collision_check_box->Size = System::Drawing::Size(15, 14);
			this->draw_collision_check_box->TabIndex = 8;
			this->draw_collision_check_box->UseVisualStyleBackColor = true;
			this->draw_collision_check_box->CheckedChanged += gcnew System::EventHandler(this, &options_page_general::draw_collision_check_box_CheckedChanged);
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(3, 111);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(59, 13);
			this->label6->TabIndex = 10;
			this->label6->Text = L"Near plane";
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(3, 137);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(51, 13);
			this->label7->TabIndex = 11;
			this->label7->Text = L"Far plane";
			// 
			// m_near_plane_box
			// 
			this->m_near_plane_box->DecimalPlaces = 1;
			this->m_near_plane_box->Location = System::Drawing::Point(108, 111);
			this->m_near_plane_box->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {4, 0, 0, 0});
			this->m_near_plane_box->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->m_near_plane_box->Name = L"m_near_plane_box";
			this->m_near_plane_box->Size = System::Drawing::Size(120, 20);
			this->m_near_plane_box->TabIndex = 14;
			this->m_near_plane_box->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {4, 0, 0, 0});
			this->m_near_plane_box->ValueChanged += gcnew System::EventHandler(this, &options_page_general::m_near_plane_box_ValueChanged);
			// 
			// m_far_plane_box
			// 
			this->m_far_plane_box->DecimalPlaces = 1;
			this->m_far_plane_box->Location = System::Drawing::Point(108, 137);
			this->m_far_plane_box->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10000, 0, 0, 0});
			this->m_far_plane_box->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			this->m_far_plane_box->Name = L"m_far_plane_box";
			this->m_far_plane_box->Size = System::Drawing::Size(120, 20);
			this->m_far_plane_box->TabIndex = 15;
			this->m_far_plane_box->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {3005, 0, 0, 65536});
			this->m_far_plane_box->ValueChanged += gcnew System::EventHandler(this, &options_page_general::m_near_plane_box_ValueChanged);
			// 
			// m_focus_distance_box
			// 
			this->m_focus_distance_box->DecimalPlaces = 1;
			this->m_focus_distance_box->Location = System::Drawing::Point(108, 175);
			this->m_focus_distance_box->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {2, 0, 0, 0});
			this->m_focus_distance_box->Name = L"m_focus_distance_box";
			this->m_focus_distance_box->Size = System::Drawing::Size(120, 20);
			this->m_focus_distance_box->TabIndex = 17;
			this->m_focus_distance_box->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {100, 0, 0, 0});
			this->m_focus_distance_box->ValueChanged += gcnew System::EventHandler(this, &options_page_general::m_focus_distance_box_ValueChanged);
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(3, 175);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(79, 13);
			this->label8->TabIndex = 16;
			this->label8->Text = L"Focus distance";
			// 
			// options_page_general
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->m_focus_distance_box);
			this->Controls->Add(this->label8);
			this->Controls->Add(this->m_far_plane_box);
			this->Controls->Add(this->m_near_plane_box);
			this->Controls->Add(this->label7);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->draw_collision_check_box);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->draw_grid_check_box1);
			this->Name = L"options_page_general";
			this->Size = System::Drawing::Size(236, 246);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_near_plane_box))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_far_plane_box))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_focus_distance_box))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
		System::Collections::Hashtable^		m_backup_values;
		editor_world&						m_editor_world;
	private: System::Void draw_grid_check_box1_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	private: System::Void draw_collision_check_box_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	private: System::Void m_near_plane_box_ValueChanged(System::Object^  sender, System::EventArgs^  e);
	private: System::Void m_focus_distance_box_ValueChanged(System::Object^  sender, System::EventArgs^  e);
};

} //	namespace editor
} //	namespace xray
