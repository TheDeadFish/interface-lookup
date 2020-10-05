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


		xArray<char*> funcs;
		xArray<ArgsSet> argSet;
	};

	xArray<Interface> iLst;

	int load(cch* file);
	xarray<Interface*> find(char* str);
};
