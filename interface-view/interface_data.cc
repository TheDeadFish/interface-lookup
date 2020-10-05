#include <stdshit.h>
#include "interface_data.h"

int InterfaceData::load(cch* file)
{
	#define LINE_GET() *++curPos

	// load text file
	int nLines;
	char** lines = loadText(file, nLines);
	if(lines == NULL) return 1;
	SCOPE_EXIT(lines);

	// loop over lines
	char** curPos = lines;
	char* type = strtok(*curPos, ",");
	while(type && (type[0] == '#'))
	{
		// create interface
		printf("%s\n", type);
		auto& inter = iLst.xnxcalloc();
		inter.type = type+1;
		inter.base = strtok(NULL, ",");

		// get functions
		while(char* name = strtok(NULL, ",")) {
			inter.funcs.push_back(name); }

		do {
			auto& argSet = inter.argSet.xnxcalloc();
			argSet.files = LINE_GET();
			if(!argSet.files) return 2;

			argSet.x.init(xMalloc(inter.funcs.len));
			for(int i = 0; i < inter.funcs.len; i++) {
				xarray<char*> tmp = {};
				char* line = strtok(LINE_GET(), ",");
				if(!line) return 2; tmp.push_back(line);
				while(char* arg = strtok(NULL, ","))
					tmp.push_back(arg);
				tmp.push_back((char*)0);
				argSet.x[i].init(tmp.data);
			}

			// advance position
			type = strtok(LINE_GET(), ",");

		} while(type && (type[0] != '#'));
	}

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
