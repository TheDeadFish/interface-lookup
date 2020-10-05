#include <stdshit.h>
#include "findfile.h"
#include "c-parse.h"

const char progName[] = "interface-grep";

static FILE* s_fpOut;
static byte s_prevTok;
static int s_printFlag;
static cParse* s_cParse;
static const char* s_curFile;
static xVector<cParse::Parse_t> s_args;


void error(char* str, cch* fmt, ...)
{
	va_list va; va_start (va, fmt);
	vprintf (fmt, va);

	printf(" @%s:%d:%d\n", s_curFile,
		s_cParse->getLine(str));
}

struct defineName_t {
	int type; cstr name; 
};

struct define_t {
	cstr name; int nArgs;
	cParse::Parse_t toks;
};

xarray<define_t> defList;

defineName_t parse_defName(
	cParse::Parse_t& pos)
{
	// get call
	defineName_t ret = {-1,
		pos.getCall2(s_args)};
	if(ret.name.slen) {
		ret.type = cParse::fixArgs(s_args);
		if(ret.type > 0) ret.type = s_args.size()+1;
	} return ret;
}

define_t* lookup_define(defineName_t& name)
{
	int nArgs = name.type-1;

	for(auto& def : defList) {
		if(def.nArgs == nArgs
		&&(!def.name.cmp(name.name)))
			return &def;
	} return NULL;
}

void parse_define(cParse::Parse_t pos)
{
	// get define name
	pos.data += 2;
	auto name = parse_defName(pos);
	if(name.type < 0) return;

	// skip defines without STDMETHOD_
	if(!pos.text().str("STDMETHOD_"))
		return;

	// validate define
	if(!pos.defineInit(s_args)) {
		error(name.name, "bad define");
		return; }

	// lookup/allocate item
	define_t* x = lookup_define(name);
	if(!x) { x = &defList.xnxcalloc(); 
		x->name = name.name; }

	// update item
	x->toks = pos;
	x->nArgs = name.type-1;
	//printf("%d, %.*s\n", name.type-1, name.name.prn());
}

void print_tokens(cParse::Parse_t x, int flags)
{
	if(flags & 1) fputs(", ", s_fpOut);
	if(flags & 2) fputs("[", s_fpOut);
	fputs("\"", s_fpOut); x.print(s_fpOut, 0);
	fputs("\"", s_fpOut);	
	if(flags & 4) fputs(", [", s_fpOut);
}

void print_fileName(cch* name)
{
	// escape the filename
	Bstr str;
	for(int i = 0; name[i]; i++) {
		if(name[i] == '\\')
			str.push_back(name[i]);
		str.push_back(name[i]);
	}

	fprintf(s_fpOut, ", \"%s\", [", str.data);
}

void idl_attrib(cParse::Parse_t& toks)
{
	int flags = 0;
	while(toks->value() == CTOK_LQBR) {
		auto atribs = toks.splitR(CTOK_RQBR);

		// find in/out attributes
		for(auto& tok : atribs) {
			if(!tok.cStr().cmp("out")) flags |= 1;
			if(!tok.cStr().cmp("in")) flags |= 2;
		}
	}

	// prepend IN/OUT tokens
	if(flags & 1) { toks.data--;
		*toks = cParse::Token::make(CTOK_NAME, "OUT"); }
	if(flags & 2) { toks.data--;
		*toks = cParse::Token::make(CTOK_NAME, "IN"); }
}

bool parse_methods(bool idlMode, cParse::Parse_t pos)
{
	static const cParse::Token hresult = 
		cParse::Token::make(CTOK_NAME, "HRESULT");

	while(pos.chk())
	{
		// skip operators
		if(pos->value() != CTOK_NAME) {

			// skip idl attributes
			if((idlMode)
			&&(pos->value() == CTOK_LQBR)) {
				pos.splitR(CTOK_RQBR); continue; }

			// skip operator
			pos.fi(); continue;
		}

		// parse name
		auto item = parse_defName(pos);
		if(item.type < 0) {
			error(pos->str, "bad args");
			return false; }

		if(idlMode)
		{
			// validate function type
			if(item.type != 0) {
				if(!item.name.cmp("cpp_quote")) continue; ERR:
				error(pos->str, "bad idl function");
				return false; }

			// skip typedef, enum, const
			if((!item.name.cmp("typedef"))
			||(!item.name.cmp("enum"))
			||(!item.name.cmp("const"))
			||(!item.name.cmp("struct"))) {
				pos.splitR_stmtEnd(); continue; }

			// skip virtual
			if(!pos[-1].cStr().cmp("virtual")) {
				if(!pos.chk()) goto ERR; pos.fi(); }

			// get idl function type
			s_args.xresize(2);
			s_args[0] = {&pos[-1],1};
			while(1) { if(!pos.chk()) goto ERR;
				if(pos->value() != CTOK_MUL) break;
				s_args[0].end_++; pos.fi(); }

			// get idl function name
			if(!pos[0].cStr().cmp("STDMETHODCALLTYPE")) {
				pos.fi(); if(!pos.chk()) goto ERR; }
			if(pos[0].value() != CTOK_NAME) goto ERR;
			s_args[1] = {&pos[0],1}; pos.fi();

		} else {

			// STDMETHOD_
			if((s_args.size() != 2)
			||(item.name.cmp("STDMETHOD_"))) {
			
			
				// STDMETHOD
				if((s_args.size() == 1)
				&&((!item.name.cmp("STDMETHOD"))
				||(!item.name.cmp("STDMETHODV")))) {
					s_args.xresize(2);
					s_args[1] = s_args[0];
					s_args[0] = {(cParse::Token*)&hresult, 1};
					
				} else {
				
					// filter names
					if((!item.name.cmp("PURE"))
					||(!item.name.cmp("DEFINE_ABSTRACT_UNKNOWN"))
					||(!item.name.cmp("public"))
					||(!item.name.cmp("BEGIN_INTERFACE"))
					||(!item.name.cmp("END_INTERFACE")))
						continue;

					auto* macro = lookup_define(item);
					if(macro) parse_methods(false, macro->toks);
					else { error(item.name, "bad function: %.*s", 
						item.name.prn()); }
					continue;
				}
			}
		}
			
			// method type and name
			print_tokens(s_args[0], s_printFlag);
			s_printFlag = 3;
			print_tokens(s_args[1], 5);
			
			// method arguments
			pos.getArgs(s_args);
			for(auto& x : s_args)
			{
				// skip THIS argument
				if(!x->cStr().cmp("THIS_")
				|| !x->cStr().cmp("THIS")) x.fi();
				if(!x.chk()) continue;
				
				// print argument
				idl_attrib(x);
				print_tokens(x, s_args.data() != &x);
			}

			fputs("]]", s_fpOut);
	}

	return true;
}

void parse_interface(cch* name)
{
	cParse cp;
	
	if(!cp.load2(name, 0)) {
	
		s_cParse = &cp;
		s_curFile = name;
		

		auto pos = cp.tokLst;
		while(pos.chk())
		{
			// parse any macros
			if(pos.cppType() == CPP_DEFINE) {
				parse_define(pos.cppBlock());;
				continue;
			}
			
			// skip operators
			if(pos->value() != CTOK_NAME) {
				pos.fi(); continue; }

			// idl interface
			bool idlMode = false;
			if(!pos->cStr().cmp("interface"))
			{
				// validate idl
				pos.fi(); if((!pos.chk(3))
				||(pos[0].value() != CTOK_NAME)
				||(pos[1].value() != CTOK_COLON)
				||(pos[2].value() != CTOK_NAME))
					continue;

				// get idl class type
				s_args.xresize(2);
				s_args[0] = {&pos[0],1};

				// skip public
				if(!pos[2].cStr().cmp("public")) {
					pos.fi();
					if(pos[2].value() != CTOK_NAME)
						continue;
				}

				// get idl base type
				s_args[1] = {&pos[2],1};
				idlMode = true;
				pos.data += 3;

			} else {

				// get interface
				cstr name = pos.getCall2(s_args);
				if(s_args.size() != 2) continue;
				if(name.cmp("DECLARE_INTERFACE_"))
					continue;
			}
				
			// print interface
			size_t prevPos = ftell(s_fpOut);
			print_tokens(s_args[0], 2);
			print_tokens(s_args[1], 1);
			print_fileName(name);
		
			// parse methods
			s_printFlag = 2;
			if(parse_methods(idlMode, pos.splitR_brClose(CTOK_LCBR)))
				fputs("]],\n", s_fpOut);
			else fseek(s_fpOut, prevPos, SEEK_SET);
		}
		
		defList.Clear();
		
	}
}

size_t __stdcall findcb(int, FindFiles_t& ff)
{
	if(!strcmp(ff.cFileName, "interface-grep.txt"))
		return 0;
	parse_interface(ff.fName.data);

	return 0;
}

int main(int argc, char* argv[])
{
	char* fName = pathCatF(getProgramDir(), "interface_list.py");
	s_fpOut = fopen(fName, "w");
	fputs("interface = [\n", s_fpOut);
	findFiles(".", 0, 0, findcb);
	fputs("]", s_fpOut);
	return 0;
}
