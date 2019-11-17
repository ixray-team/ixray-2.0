////////////////////////////////////////////////////////////////////////////
//	Module 		: world.h
//	Created 	: 10.04.2008
//  Modified 	: 10.04.2008
//	Author		: Dmitriy Iassenev
//	Description : lua studio backend world class
////////////////////////////////////////////////////////////////////////////

#ifndef CS_LUA_STUDIO_BACKEND_WORLD_H_INCLUDED
#define CS_LUA_STUDIO_BACKEND_WORLD_H_INCLUDED

struct lua_State;

namespace cs {

struct message_initiator
{
};

struct DECLSPEC_NOVTABLE core
{
//virtual void CS_SCRIPT_CALL	message_type	(const message_type &message_type);
};

namespace script {

struct engine;

struct DECLSPEC_NOVTABLE world {
	virtual	void CS_SCRIPT_CALL	add					(lua_State *state) = 0;
	virtual	void CS_SCRIPT_CALL	remove				(lua_State *state) = 0;
	virtual	int  CS_SCRIPT_CALL	on_error			(lua_State *state) = 0;
	virtual	void CS_SCRIPT_CALL	add_log_line		(const char *log_line) = 0;
	virtual	bool CS_SCRIPT_CALL	evaluating_watch	() = 0;
}; // struct DECLSPEC_NOVTABLE world

struct DECLSPEC_NOVTABLE file_handle {
};

typedef void*	maf_parameter;
typedef void*	(CS_SCRIPT_CALL *maf_ptr) (maf_parameter parameter, void const *, size_t);

typedef world*	(CS_SCRIPT_CALL *create_world_function_type)	(engine& engine, bool, bool);
typedef void	(CS_SCRIPT_CALL *destroy_world_function_type)	(world*& world);
typedef void	(CS_SCRIPT_CALL *memory_allocator_function_type)(maf_ptr memory_allocator, maf_parameter parameter);
typedef size_t	(CS_SCRIPT_CALL *memory_stats_function_type)	();

} // namespace script
} // namespace cs

extern "C" {
	CS_LUA_STUDIO_BACKEND_API	cs::script::world*			CS_SCRIPT_CALL	cs_script_create_world				(cs::script::engine& engine, bool use_bugtrap, bool create_log_file);
	CS_LUA_STUDIO_BACKEND_API	void						CS_SCRIPT_CALL	cs_script_destroy_world				(cs::script::world*& world);

	CS_LUA_STUDIO_BACKEND_API	void						CS_SCRIPT_CALL	memory_allocator			(cs::script::maf_ptr memory_allocator, cs::script::maf_parameter parameter);
	CS_LUA_STUDIO_BACKEND_API	size_t						CS_SCRIPT_CALL	memory_stats				();
}

#endif // #ifndef CS_LUA_STUDIO_BACKEND_WORLD_H_INCLUDED