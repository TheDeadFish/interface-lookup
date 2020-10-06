#include <stdshit.h>
#include "interface_data.h"

static
int compar(const InterfaceData::Interface& a, const InterfaceData::Interface& b) {
	return strcmp(a.type, b.type); }
static
int findFn(cch* pkey, const InterfaceData::Interface& elem) {
	return strcmp(pkey, elem.type); }

int InterfaceData::load(cch* file)
{
	// load text file
	int nLines;
	char** lines = loadText(file, nLines);
	if(lines == NULL) return 1;
	SCOPE_EXIT(lines);

	// loop over lines
	char** curPos = lines;
	while(1)
	{
		char* type = strtok(*curPos++, ",");
		if(!type || (type[0] != '#')) break;

		// create interface
		auto& inter = iLst.xnxcalloc();
		inter.type = type+1;
		inter.base = strtok(NULL, ",");

		// get functions
		while(char* name = strtok(NULL, ",")) {
			inter.funcs.push_back(name); }

		while(1) {
			auto& argSet = inter.argSet.xnxcalloc();
			argSet.files = *curPos++;
			if(!argSet.files) return 2;

			argSet.x.init(xMalloc(inter.funcs.len));
			for(int i = 0; i < inter.funcs.len; i++) {
				xarray<char*> tmp = {};
				char* line = strtok(*curPos++, ",");
				if(!line) return 2; tmp.push_back(line);
				while(char* arg = strtok(NULL, ","))
					tmp.push_back(arg);
				tmp.push_back((char*)0);
				argSet.x[i].init(tmp.data);
			}

			if(!*curPos ||(*curPos)[0] == '#')
				break;
		}
	}

	// initialize pBase pointers
	qsort(iLst.data, iLst.len, compar);
	for(auto& inter : iLst) {
		inter.pBase = find_exact(inter.base); }

	return 0;
}

xarray<InterfaceData::Interface*> InterfaceData::find(char* str_)
{
	xarray<Interface*> lst = {};
	cstr str = str_;
	if(str.slen) {
	for(auto& inter : iLst) {
		if(cstr(inter.type).istr(str))
			lst.push_back(&inter);
	}}
	return lst;
}



InterfaceData::Interface* InterfaceData::find_exact(char* str)
{
	return bsearch((void*)str, iLst.data, iLst.len, findFn);
}

xarray<char*> InterfaceData::Interface::getArgs(int iFunc)
{
	char** args = argSet[setSel].x[iFunc];
	assert(args[0] != NULL);
	xarray<char*> ret = {args, 1};
	while(*ret.end()) ret.len++;
	return ret;
}

void InterfaceData::fmtArg(bstr& str, cch* curPos)
{
	while(*curPos) {

		// locate pointer
		cch* basePos = curPos;
		while(*curPos) { curPos++;
			if((curPos[-1] == '*')
			&&(curPos[0] != '*'))
				break;
		}


		str.fmtcat("%v ", cstr(basePos, curPos));
	}

	if(str.slen) { str.slen--;
		str.fmtcat(", "); }
}

void InterfaceData::Interface::fmtFunc(bstr& str, int iFunc)
{
	auto args = getArgs(iFunc);
	str.fmtcat("%s %s(", args[0], funcs[iFunc]);
	for(char* arg : args.right(1)) fmtArg(str, arg);
	if(args.len > 1) str.slen -= 2;
	str.fmtcat(");");
}

void InterfaceData::Interface::fmtFuncS(bstr& str, int iFunc)
{
	auto args = getArgs(iFunc);
	str.fmtcat("%s@%d", funcs[iFunc], args.len-1);
}

int InterfaceData::Interface::fmtFuncs(bstr& str, int offset, FmtConf& fc)
{
	if(pBase) offset = pBase->fmtFuncs(str, offset, fc);
	str.fmtcat("  // %s methods", type);
	for(int i = 0; i < funcs.len; i++) {
		if(fc.simple == 2) {
			str.fmtcat("\n  void* %s;", funcs[i]);
		} else {
			str.fmtcat("\n  /* %02X */  ", offset); offset += fc.pSize;
			if(fc.simple) fmtFuncS(str, i); else fmtFunc(str, i);
		}
	}
	str.fmtcat("\n\n");
	return offset;
}
