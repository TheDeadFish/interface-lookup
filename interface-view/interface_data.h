#pragma once

struct InterfaceData
{
	struct Interface
	{
		char* type;
		char* base;

		struct ArgsSet {
			char* fileName;
			char* type;
			xarray<char*> arg;
		};



		xarray<ArgsSet> argSet;
	};

	xarray<Interface> iLst;

	int load(cch* file);
};
