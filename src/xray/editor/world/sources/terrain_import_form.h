#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace xray{
namespace editor {
	ref struct terrain_import_settings;

	/// <summary>
	/// Summary for terrain_import_form
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class terrain_import_form : public System::Windows::Forms::Form
	{
	public:
		terrain_import_form(void)
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
		~terrain_import_form()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::GroupBox^  groupBox1;
	protected: 
	private: System::Windows::Forms::Button^  select_source_button;
	private: System::Windows::Forms::TextBox^  source_text_box;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::Button^  ok_button;
	private: System::Windows::Forms::Button^  cancel_button;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::RadioButton^  diffuse_rgb_radio;

	private: System::Windows::Forms::RadioButton^  diffuse_rgba_radio;
	private: System::Windows::Forms::CheckBox^  create_cell_if_empty_check;
	private: System::Windows::Forms::RadioButton^  diffuse_rgb_height_a_radio;
	private: System::Windows::Forms::RadioButton^  height_a_radio;

	private: System::Windows::Forms::Label^  file_desc_label;
	private: System::Windows::Forms::CheckBox^  stretch_image_check;
	private: System::Windows::Forms::TrackBar^  height_scale_track_bar;
	private: System::Windows::Forms::TextBox^  height_scale_value_text_box;
	private: System::Windows::Forms::Label^  label1;


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
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->file_desc_label = (gcnew System::Windows::Forms::Label());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->height_scale_track_bar = (gcnew System::Windows::Forms::TrackBar());
			this->height_scale_value_text_box = (gcnew System::Windows::Forms::TextBox());
			this->height_a_radio = (gcnew System::Windows::Forms::RadioButton());
			this->diffuse_rgb_height_a_radio = (gcnew System::Windows::Forms::RadioButton());
			this->diffuse_rgb_radio = (gcnew System::Windows::Forms::RadioButton());
			this->diffuse_rgba_radio = (gcnew System::Windows::Forms::RadioButton());
			this->select_source_button = (gcnew System::Windows::Forms::Button());
			this->source_text_box = (gcnew System::Windows::Forms::TextBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->stretch_image_check = (gcnew System::Windows::Forms::CheckBox());
			this->create_cell_if_empty_check = (gcnew System::Windows::Forms::CheckBox());
			this->ok_button = (gcnew System::Windows::Forms::Button());
			this->cancel_button = (gcnew System::Windows::Forms::Button());
			this->groupBox1->SuspendLayout();
			this->groupBox3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->height_scale_track_bar))->BeginInit();
			this->groupBox2->SuspendLayout();
			this->SuspendLayout();
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->file_desc_label);
			this->groupBox1->Controls->Add(this->groupBox3);
			this->groupBox1->Controls->Add(this->select_source_button);
			this->groupBox1->Controls->Add(this->source_text_box);
			this->groupBox1->Dock = System::Windows::Forms::DockStyle::Top;
			this->groupBox1->Location = System::Drawing::Point(0, 0);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(487, 141);
			this->groupBox1->TabIndex = 0;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"source";
			// 
			// file_desc_label
			// 
			this->file_desc_label->AutoSize = true;
			this->file_desc_label->Location = System::Drawing::Point(10, 42);
			this->file_desc_label->Name = L"file_desc_label";
			this->file_desc_label->Size = System::Drawing::Size(78, 13);
			this->file_desc_label->TabIndex = 3;
			this->file_desc_label->Text = L"no file selected";
			// 
			// groupBox3
			// 
			this->groupBox3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->groupBox3->Controls->Add(this->label1);
			this->groupBox3->Controls->Add(this->height_scale_track_bar);
			this->groupBox3->Controls->Add(this->height_scale_value_text_box);
			this->groupBox3->Controls->Add(this->height_a_radio);
			this->groupBox3->Controls->Add(this->diffuse_rgb_height_a_radio);
			this->groupBox3->Controls->Add(this->diffuse_rgb_radio);
			this->groupBox3->Controls->Add(this->diffuse_rgba_radio);
			this->groupBox3->Location = System::Drawing::Point(6, 56);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(476, 79);
			this->groupBox3->TabIndex = 2;
			this->groupBox3->TabStop = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(251, 16);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(66, 13);
			this->label1->TabIndex = 6;
			this->label1->Text = L"Height scale";
			// 
			// height_scale_track_bar
			// 
			this->height_scale_track_bar->LargeChange = 100;
			this->height_scale_track_bar->Location = System::Drawing::Point(257, 34);
			this->height_scale_track_bar->Maximum = 600;
			this->height_scale_track_bar->Minimum = 1;
			this->height_scale_track_bar->Name = L"height_scale_track_bar";
			this->height_scale_track_bar->Size = System::Drawing::Size(213, 45);
			this->height_scale_track_bar->TabIndex = 5;
			this->height_scale_track_bar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->height_scale_track_bar->Value = 500;
			this->height_scale_track_bar->Scroll += gcnew System::EventHandler(this, &terrain_import_form::height_scale_track_bar_Scroll);
			// 
			// height_scale_value_text_box
			// 
			this->height_scale_value_text_box->Location = System::Drawing::Point(323, 13);
			this->height_scale_value_text_box->Name = L"height_scale_value_text_box";
			this->height_scale_value_text_box->ReadOnly = true;
			this->height_scale_value_text_box->Size = System::Drawing::Size(146, 20);
			this->height_scale_value_text_box->TabIndex = 4;
			// 
			// height_a_radio
			// 
			this->height_a_radio->AutoSize = true;
			this->height_a_radio->Location = System::Drawing::Point(109, 42);
			this->height_a_radio->Name = L"height_a_radio";
			this->height_a_radio->Size = System::Drawing::Size(69, 17);
			this->height_a_radio->TabIndex = 3;
			this->height_a_radio->Text = L"Height(A)";
			this->height_a_radio->UseVisualStyleBackColor = true;
			// 
			// diffuse_rgb_height_a_radio
			// 
			this->diffuse_rgb_height_a_radio->AutoSize = true;
			this->diffuse_rgb_height_a_radio->Location = System::Drawing::Point(109, 19);
			this->diffuse_rgb_height_a_radio->Name = L"diffuse_rgb_height_a_radio";
			this->diffuse_rgb_height_a_radio->Size = System::Drawing::Size(134, 17);
			this->diffuse_rgb_height_a_radio->TabIndex = 2;
			this->diffuse_rgb_height_a_radio->Text = L"Diffuse(RGB) Height(A)";
			this->diffuse_rgb_height_a_radio->UseVisualStyleBackColor = true;
			// 
			// diffuse_rgb_radio
			// 
			this->diffuse_rgb_radio->AutoSize = true;
			this->diffuse_rgb_radio->Location = System::Drawing::Point(6, 42);
			this->diffuse_rgb_radio->Name = L"diffuse_rgb_radio";
			this->diffuse_rgb_radio->Size = System::Drawing::Size(87, 17);
			this->diffuse_rgb_radio->TabIndex = 1;
			this->diffuse_rgb_radio->Text = L"Diffuse(RGB)";
			this->diffuse_rgb_radio->UseVisualStyleBackColor = true;
			// 
			// diffuse_rgba_radio
			// 
			this->diffuse_rgba_radio->AutoSize = true;
			this->diffuse_rgba_radio->Checked = true;
			this->diffuse_rgba_radio->Location = System::Drawing::Point(6, 19);
			this->diffuse_rgba_radio->Name = L"diffuse_rgba_radio";
			this->diffuse_rgba_radio->Size = System::Drawing::Size(97, 17);
			this->diffuse_rgba_radio->TabIndex = 0;
			this->diffuse_rgba_radio->TabStop = true;
			this->diffuse_rgba_radio->Text = L"Diffuse (RGBA)";
			this->diffuse_rgba_radio->UseVisualStyleBackColor = true;
			// 
			// select_source_button
			// 
			this->select_source_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->select_source_button->Location = System::Drawing::Point(459, 19);
			this->select_source_button->Name = L"select_source_button";
			this->select_source_button->Size = System::Drawing::Size(23, 23);
			this->select_source_button->TabIndex = 1;
			this->select_source_button->Text = L"...";
			this->select_source_button->UseVisualStyleBackColor = true;
			this->select_source_button->Click += gcnew System::EventHandler(this, &terrain_import_form::select_source_button_Click);
			// 
			// source_text_box
			// 
			this->source_text_box->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->source_text_box->Location = System::Drawing::Point(6, 19);
			this->source_text_box->Name = L"source_text_box";
			this->source_text_box->ReadOnly = true;
			this->source_text_box->Size = System::Drawing::Size(447, 20);
			this->source_text_box->TabIndex = 0;
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->stretch_image_check);
			this->groupBox2->Controls->Add(this->create_cell_if_empty_check);
			this->groupBox2->Dock = System::Windows::Forms::DockStyle::Top;
			this->groupBox2->Location = System::Drawing::Point(0, 141);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(487, 53);
			this->groupBox2->TabIndex = 1;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"options";
			// 
			// stretch_image_check
			// 
			this->stretch_image_check->AutoSize = true;
			this->stretch_image_check->Location = System::Drawing::Point(127, 19);
			this->stretch_image_check->Name = L"stretch_image_check";
			this->stretch_image_check->Size = System::Drawing::Size(117, 17);
			this->stretch_image_check->TabIndex = 1;
			this->stretch_image_check->Text = L"Stretch to selection";
			this->stretch_image_check->UseVisualStyleBackColor = true;
			// 
			// create_cell_if_empty_check
			// 
			this->create_cell_if_empty_check->AutoSize = true;
			this->create_cell_if_empty_check->Location = System::Drawing::Point(6, 19);
			this->create_cell_if_empty_check->Name = L"create_cell_if_empty_check";
			this->create_cell_if_empty_check->Size = System::Drawing::Size(115, 17);
			this->create_cell_if_empty_check->TabIndex = 0;
			this->create_cell_if_empty_check->Text = L"Create cell if empty";
			this->create_cell_if_empty_check->UseVisualStyleBackColor = true;
			// 
			// ok_button
			// 
			this->ok_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->ok_button->Location = System::Drawing::Point(325, 194);
			this->ok_button->Name = L"ok_button";
			this->ok_button->Size = System::Drawing::Size(75, 24);
			this->ok_button->TabIndex = 2;
			this->ok_button->Text = L"Ok";
			this->ok_button->UseVisualStyleBackColor = true;
			this->ok_button->Click += gcnew System::EventHandler(this, &terrain_import_form::ok_button_Click);
			// 
			// cancel_button
			// 
			this->cancel_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->cancel_button->Location = System::Drawing::Point(406, 194);
			this->cancel_button->Name = L"cancel_button";
			this->cancel_button->Size = System::Drawing::Size(75, 24);
			this->cancel_button->TabIndex = 3;
			this->cancel_button->Text = L"Cancel";
			this->cancel_button->UseVisualStyleBackColor = true;
			this->cancel_button->Click += gcnew System::EventHandler(this, &terrain_import_form::cancel_button_Click);
			// 
			// terrain_import_form
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(487, 223);
			this->Controls->Add(this->cancel_button);
			this->Controls->Add(this->ok_button);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"terrain_import_form";
			this->Text = L"Import";
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->height_scale_track_bar))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void select_source_button_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void ok_button_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void cancel_button_Click(System::Object^  sender, System::EventArgs^  e);
	public:
		terrain_import_settings^			m_settings;
	private:
		void								in_constructor();
	private: System::Void height_scale_track_bar_Scroll(System::Object^  sender, System::EventArgs^  e);
};

} // namespace editor
} // namespace xray