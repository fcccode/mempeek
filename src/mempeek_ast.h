#ifndef __mempeek_ast_h__
#define __mempeek_ast_h__

#include "mempeek_parser.h"
#include "environment.h"

#include <ostream>
#include <string>
#include <vector>

#include <stdint.h>

#ifdef ASTDEBUG
#include <iostream>
#endif


//////////////////////////////////////////////////////////////////////////////
// class ASTNode
//////////////////////////////////////////////////////////////////////////////

class ASTNode {
public:
	typedef struct {
		std::string file;
		int first_line;
		int last_line;
	} location_t;

	ASTNode( const yylloc_t* yylloc );
	virtual ~ASTNode();

	const ASTNode::location_t& get_location();

	void add_child( ASTNode* node );

	virtual uint64_t execute() = 0;

	static ASTNode* parse( const char* str, bool is_file );
	static ASTNode* parse( const location_t& location, const char* str, bool is_file );

	static int get_default_size();

	static Environment& get_environment();

	static void add_include_path( std::string path );

	static void set_terminate();
    static void clear_terminate();
	static bool is_terminated();

protected:
	typedef std::vector< ASTNode* > nodelist_t;

	const nodelist_t& get_children();

	static uint64_t parse_int( std::string str );

private:
	nodelist_t m_Children;

	location_t m_Location;

	static Environment s_Environment;
	static volatile bool s_IsTerminated;

	static std::vector< std::string > s_IncludePaths;

	ASTNode( const ASTNode& ) = delete;
	ASTNode& operator=( const ASTNode& ) = delete;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeBreak
//////////////////////////////////////////////////////////////////////////////

class ASTNodeBreak : public ASTNode {
public:
    ASTNodeBreak( const yylloc_t* yylloc, int token );

    uint64_t execute() override;

private:
    int m_Token;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeBlock
//////////////////////////////////////////////////////////////////////////////

class ASTNodeBlock : public ASTNode {
public:
	ASTNodeBlock( const yylloc_t* yylloc );

	uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeAssign
//////////////////////////////////////////////////////////////////////////////

class ASTNodeAssign : public ASTNode {
public:
    ASTNodeAssign( const yylloc_t* yylloc, std::string name, ASTNode* expression );

    uint64_t execute() override;

    Environment::var* get_var();

private:
    Environment::var* m_Var;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeIf
//////////////////////////////////////////////////////////////////////////////

class ASTNodeIf : public ASTNode {
public:
	ASTNodeIf( const yylloc_t* yylloc, ASTNode* condition, ASTNode* then_block );
	ASTNodeIf( const yylloc_t* yylloc, ASTNode* condition, ASTNode* then_block, ASTNode* else_block  );

	uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeWhile
//////////////////////////////////////////////////////////////////////////////

class ASTNodeWhile : public ASTNode {
public:
	ASTNodeWhile( const yylloc_t* yylloc, ASTNode* condition, ASTNode* block );

	uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeFor
//////////////////////////////////////////////////////////////////////////////

class ASTNodeFor : public ASTNode {
public:
    ASTNodeFor( const yylloc_t* yylloc, ASTNodeAssign* var, ASTNode* to );
    ASTNodeFor( const yylloc_t* yylloc, ASTNodeAssign* var, ASTNode* to, ASTNode* step );

    uint64_t execute() override;

private:
    Environment::var* m_Var;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodePeek
//////////////////////////////////////////////////////////////////////////////

class ASTNodePeek : public ASTNode {
public:
	ASTNodePeek( const yylloc_t* yylloc, ASTNode* address, int size_restriction );

	uint64_t execute() override;

private:
	template< typename T> uint64_t peek();

	int m_SizeRestriction;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodePoke
//////////////////////////////////////////////////////////////////////////////

class ASTNodePoke : public ASTNode {
public:
	ASTNodePoke( const yylloc_t* yylloc, ASTNode* address, ASTNode* value, int size_restriction );
	ASTNodePoke( const yylloc_t* yylloc, ASTNode* address, ASTNode* value, ASTNode* mask, int size_restriction );

	uint64_t execute() override;

private:
	template< typename T> void poke();

	int m_SizeRestriction;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodePrint
//////////////////////////////////////////////////////////////////////////////

class ASTNodePrint : public ASTNode {
public:
	enum {
		MOD_DEC = 0x01,
		MOD_HEX = 0x02,
		MOD_BIN = 0x03,
		MOD_NEG = 0x04,

		MOD_8BIT = 0x10,
		MOD_16BIT = 0x20,
		MOD_32BIT = 0x30,
		MOD_64BIT = 0x40,

		MOD_SIZEMASK = 0xf0,
		MOD_TYPEMASK = 0x0f
	};

	ASTNodePrint( const yylloc_t* yylloc );
	ASTNodePrint( const yylloc_t* yylloc, std::string text );
	ASTNodePrint( const yylloc_t* yylloc, ASTNode* expression, int modifier );

	uint64_t execute() override;

	static int get_default_size();

private:
	void print_value( std::ostream& out, uint64_t value );

	int m_Modifier = MOD_DEC | MOD_32BIT;
	std::string m_Text = "";
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeSleep
//////////////////////////////////////////////////////////////////////////////

class ASTNodeSleep : public ASTNode {
public:
    ASTNodeSleep( const yylloc_t* yylloc, ASTNode* expression );

    uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeDef
//////////////////////////////////////////////////////////////////////////////

class ASTNodeDef : public ASTNode {
public:
	ASTNodeDef( const yylloc_t* yylloc, std::string name, ASTNode* address );
	ASTNodeDef( const yylloc_t* yylloc, std::string name, ASTNode* address, std::string from );

	uint64_t execute() override;

private:
	Environment::var* m_Def;

	const Environment::var* m_FromBase;
	std::vector< std::pair< Environment::var*, const Environment::var* > > m_FromMembers;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeMap
//////////////////////////////////////////////////////////////////////////////

class ASTNodeMap : public ASTNode {
public:
	ASTNodeMap( const yylloc_t* yylloc, std::string address, std::string size );
    ASTNodeMap( const yylloc_t* yylloc, std::string address, std::string size, std::string device );

	uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeImport
//////////////////////////////////////////////////////////////////////////////

class ASTNodeImport : public ASTNode {
public:
	ASTNodeImport( const yylloc_t* yylloc, std::string file );

	uint64_t execute() override;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeUnaryOperator
//////////////////////////////////////////////////////////////////////////////

class ASTNodeUnaryOperator : public ASTNode {
public:
	ASTNodeUnaryOperator( const yylloc_t* yylloc, ASTNode* expression, int op );

	uint64_t execute() override;

private:
	int m_Operator;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeBinaryOperator
//////////////////////////////////////////////////////////////////////////////

class ASTNodeBinaryOperator : public ASTNode {
public:
	ASTNodeBinaryOperator( const yylloc_t* yylloc, ASTNode* expression1, ASTNode* expression2, int op );

	uint64_t execute() override;

private:
	int m_Operator;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeRestriction
//////////////////////////////////////////////////////////////////////////////

class ASTNodeRestriction : public ASTNode {
public:
	ASTNodeRestriction( const yylloc_t* yylloc, ASTNode* node, int size_restriction );

	uint64_t execute() override;

private:
	int m_SizeRestriction;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeVar
//////////////////////////////////////////////////////////////////////////////

class ASTNodeVar : public ASTNode {
public:
	ASTNodeVar( const yylloc_t* yylloc, std::string name );
	ASTNodeVar( const yylloc_t* yylloc, std::string name, ASTNode* index );

	uint64_t execute() override;

private:
	const Environment::var* m_Var;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeConstant
//////////////////////////////////////////////////////////////////////////////

class ASTNodeConstant : public ASTNode {
public:
	ASTNodeConstant( const yylloc_t* yylloc, std::string str );

	uint64_t execute() override;

private:
	uint64_t m_Value;
};


//////////////////////////////////////////////////////////////////////////////
// class ASTNode inline functions
//////////////////////////////////////////////////////////////////////////////

inline const ASTNode::location_t& ASTNode::get_location()
{
	return m_Location;
}

inline void ASTNode::add_child( ASTNode* node )
{
#ifdef ASTDEBUG
	std::cerr << "AST[" << this << "]: add child node=[" << node << "]" << std::endl;
#endif

	m_Children.push_back( node );
}

inline const ASTNode::nodelist_t& ASTNode::get_children()
{
	return m_Children;
}

inline Environment& ASTNode::get_environment()
{
	return s_Environment;
}

inline void ASTNode::add_include_path( std::string path )
{
    s_IncludePaths.push_back( path );
}

inline void ASTNode::set_terminate()
{
    s_IsTerminated = true;
}

inline void ASTNode::clear_terminate()
{
    s_IsTerminated = false;
}

inline bool ASTNode::is_terminated()
{
    return s_IsTerminated;
}


//////////////////////////////////////////////////////////////////////////////
// class ASTNodeAssign inline functions
//////////////////////////////////////////////////////////////////////////////

inline Environment::var* ASTNodeAssign::get_var()
{
    return m_Var;
}


#endif // __mempeek_ast_h__
