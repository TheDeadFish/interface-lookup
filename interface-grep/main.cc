#include <stdshit.h>
#include "findfile.h"
#include "c-parse.h"

const char progName[] = "interface-grep";

static FILE* s_fpOut;
static byte s_prevTok;
static int s_printFlag;

void parse_macro(cParse::Parse_t pos)
{
	





}

void print_tokens(cParse::Parse_t x, int flags)
{
	if(flags & 1) fputs(", ", s_fpOut);
	if(flags & 2) fputs("[", s_fpOut);
	fputs("\"", s_fpOut); x.print(s_fpOut, 0);
	fputs("\"", s_fpOut);	
	if(flags & 4) fputs(", [", s_fpOut);
}

void parse_methods(cParse::Parse_t pos)
{
	xVector<cParse::Parse_t> args;

	while(pos.chk())
	{
			// get function call
			cstr name = pos.getCall(args);
			if(name.slen == 0) continue;
			
			// handle macros
			if(args.size() == 0) {
				//if(!name.cmp("DEFINE_ABSTRACT_UNKNOWN")) continue;
				continue;
			}
			
			// check for STDMETHOD_
			if(args.size() != 2) continue;
			if(name.cmp("STDMETHOD_")) 
				continue; /* error? */
			
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
	s_fpOut = stdout;
	
	if(!cp.load2(name, 0)) {

		auto pos = cp.tokLst;
		while(pos.chk())
		{
			// parse any macros
			if(pos->value() == CTOK_MACRO) {
				parse_macro(pos.cppBlock());
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
	}
}

size_t __stdcall findcb(int, FindFiles_t& ff)
{
	printf("%s\n", ff.fName.data);
	parse_interface(ff.fName.data);

	return 0;
}

int main(int argc, char* argv[])
{
	findFiles(".", 0, 0, findcb);
}
