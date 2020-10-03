#include <stdshit.h>
#include "findfile.h"
#include "c-parse.h"

const char progName[] = "interface-grep";

static FILE* s_fpOut;
static byte s_prevTok;
static int s_printFlag;
static cParse* s_cParse;
const char* s_curFile;

struct defineName_t {
	int type; cstr name; 
};

struct define_t {
	defineName_t name;
	cParse::Parse_t toks; 
};

xarray<define_t> defList;

defineName_t parse_defName(cParse::Parse_t& pos)
{
	cParse::Parse_t	tmp = pos;
	defineName_t ret = {-1};

	// get define name
	if((!tmp.chk(1))
	||(tmp->value() != CTOK_NAME)) 
		return ret;
	ret.name = tmp.fi().cStr();

	// skip defines with args
	if(tmp->value() == CTOK_LBR) { if(!tmp.chk(2) 
		|| (tmp[1].value() != CTOK_RBR)) return ret;
		tmp.data += 2; ret.type++; }

	// success
	ret.type++;
	pos = tmp; return ret;
}

define_t* lookup_define(defineName_t& name)
{
	for(auto& def : defList) {
		if(def.name.type == name.type
		&&(!def.name.name.cmp(name.name)))
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

	// lookup/allocate item
	define_t* x = lookup_define(name);
	if(!x) { x = &defList.xnxcalloc(); 
		x->name = name; }

	// update item
	x->toks = pos;

}

void print_tokens(cParse::Parse_t x, int flags)
{
	if(flags & 1) fputs(", ", s_fpOut);
	if(flags & 2) fputs("[", s_fpOut);
	fputs("\"", s_fpOut); x.print(s_fpOut, 0);
	fputs("\"", s_fpOut);	
	if(flags & 4) fputs(", [", s_fpOut);
}

void error(char* str, cch* fmt, ...)
{
	va_list va; va_start (va, fmt);
	vprintf (fmt, va);

	printf("@ %d:%d\n", 
		s_cParse->getLine(str));
}

void parse_methods(cParse::Parse_t pos)
{
	xVector<cParse::Parse_t> args;

	while(pos.chk())
	{
			// skip operators
			if(pos->value() != CTOK_NAME) {
				pos.fi(); continue; }
	
			// lookup macro
			auto defName = parse_defName(pos);
			if(defName.type >= 0) {
				if((!defName.name.cmp("PURE"))
				||(!defName.name.cmp("DEFINE_ABSTRACT_UNKNOWN")))
					continue;
					
				auto* macro = lookup_define(defName);
				if(macro) parse_methods(macro->toks);
				else { error(defName.name, "unknown macro: %.*s", 
					defName.name.prn()); }
				continue;
			}

			// get function call
			cstr name = pos.getCall(args);
			if((args.size() != 2)
			||(name.cmp("STDMETHOD_"))) {
				error(pos->str, "bad function"); 
				continue;
			}
				
			
			// method type and name
			print_tokens(args[0], s_printFlag);
			s_printFlag = 3;
			print_tokens(args[1], 5);
			
			// method arguments
			pos.getArgs(args);
			for(auto& x : args) 
			{
				// skip THIS argument
				if(!x->cStr().cmp("THIS_")
				|| !x->cStr().cmp("THIS")) x.fi();
				if(!x.chk()) continue;
				
				// print argument
				print_tokens(x, args.data() != &x);
			}
			
			fputs("]]", s_fpOut);
	}
}


void parse_interface(cch* name)
{
	cParse cp;
	xVector<cParse::Parse_t> args;
	
	if(!cp.load2(name, 0)) {
	
		s_cParse = &cp;
		s_curFile = name;
		

		auto pos = cp.tokLst;
		while(pos.chk())
		{
			// parse any macros
			if(pos.cppType() == CPP_DEFINE) {
				parse_define(pos.cppBlock());
				continue;
			}
			
			// get interface
			cstr name = pos.getCall(args);
			if(args.size() != 2) continue;
			if(name.cmp("DECLARE_INTERFACE_")) 
				continue;
				
			// print interface
			print_tokens(args[0], 2);
			print_tokens(args[1], 5);
		
			// parse methods
			s_printFlag = 2;
			parse_methods(pos.splitR(CTOK_RCBR));
			fputs("]]\n", s_fpOut);
			
		}
		
		
		defList.Clear();
		
	}
}

size_t __stdcall findcb(int, FindFiles_t& ff)
{
	if(!strcmp(ff.cFileName, "interface-grep.txt"))
		return 0;

	printf("%s\n", ff.fName.data);
	parse_interface(ff.fName.data);

	return 0;
}

int main(int argc, char* argv[])
{
	s_fpOut = fopen("interface-grep.txt", "w");
	findFiles(".", 0, 0, findcb);
}
