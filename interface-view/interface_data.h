#pragma once

struct InterfaceData
{
	struct Interface
	{
		Interface* pBase;
		char* type;
		char* base;

		struct ArgsSet {
			char* files;
			xMem<xMem<char*>> x;
		};

		int setSel;


		// data access
		xarray<char*> getArgs(int iFunc);

		// text formatting
		void fmtFunc(bstr& str, int iFunc);
		int fmtFuncs(bstr& str, int offset, int pSize);




		xArray<char*> funcs;
		xArray<ArgsSet> argSet;
	};

	xArray<Interface> iLst;

	int load(cch* file);
	xarray<Interface*> find(char* str);

	Interface* find_exact(char* str);

	#define INTERFACEDATA_INTERFACE_ITER(x, ...) { \
		InterfaceData::Interface* pos = x; \
		for(;pos;pos=pos->pBase) {__VA_ARGS__; } }
};
