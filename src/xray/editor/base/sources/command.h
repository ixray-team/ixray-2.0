////////////////////////////////////////////////////////////////////////////
//	Created		: 30.03.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

namespace xray {
namespace editor_base {

//ref class command_engine;

public ref class command abstract
{
//public:
//	friend class xray::editor::command_engine;
	
public:
	virtual			~command		()	{}
	virtual bool	commit			() = 0;
	virtual void	rollback		() = 0;

	virtual void	start_preview	()	{ };
	virtual bool	end_preview		()	{ return true; };

}; // class command

}// namespace editor
}// namespace xray

#endif // #ifndef COMMAND_H_INCLUDED