#!/usr/bin/env python2

import re
from interface_list import interface;

# IUnknown interface
IUnknown = [ "IUnknown", None,
	[["HRESULT", "QueryInterface", ["REFIID riid", "void**out"]],
	["ULONG", "AddRef", []], ["ULONG", "Release", []]]]

class FuncArg:
	def __init__(self, x):
		x = re.match(r'(.*?)\s*?(\w+)?$', x.strip())
		self.type = x.group(1)
		self.name = x.group(2)

	def cmp(self, that):
		if self.type != that.type: return -1;
		if self.name != that.name: return 0;
		return 1;

class Function:
	def __init__(self, x):
		self.type = x[0]
		self.name = x[1]
		self.args = []
		for arg in x[2]:
			self.args.append(FuncArg(arg))

	def cmp(self, that):
		if self.name != that.name: return -2
		if len(self.args) != len(that.args): return -1
		if self.type != that.type: return 0
		for i in range(len(self.args)):
			diff = self.args[i].cmp(that.args[i])
			if diff <= 0: return 0
		return 1

class Interface:
	def __init__(self, x):
		self.type = x[0];
		self.base = x[1];
		self.funcs = []
		self.done = False

		for func in x[2]:
			self.funcs.append(Function(func))

		if self.base == "IUnknown":
			self.trim_funcs(["QueryInterface", "AddRef", "Release"])

	def cmp(self, that):
		if self.base != that.base: return -3
		if len(self.funcs) != len(that.funcs): return -2
		retVal = 1
		for i in range(len(self.funcs)):
			diff = self.funcs[i].cmp(that.funcs[i])
			if retVal > diff: retVal = diff
		return retVal

	def trim_funcs(self, funcList):
		i = 0
		for name in funcList:
			if i >= len(self.funcs): break;
			if self.funcs[i].name == name:
				del self.funcs[i]
			else: i += 1

	def list_funcs(self, funcList):
		for func in self.funcs:
			funcList.append(func.name)

# create interface list
interfaceDict = {"IUnknown":Interface(IUnknown)}


def add_interface(s):
	xLst = interfaceDict.setdefault(s[0], [])
	inter = Interface(s)
	for x in xLst:
		diff = inter.cmp(x)
		if diff > 0: return
		print inter.type, diff
	xLst.append(inter)

for s in interface:
	add_interface(s)









"""
def prepare_base(name, funcNames):
	if name in interfaceDict:
		print "interface not found: ", name
		return -1





	if name == "IUnknown": return 1;


	xLst = interfaceDict[name]
	for inter in xLst:
		prepare_base(inter.name, funcNames)









	# add items to list
	for func in inter.funcs:
		funcNames.append(func.name)
	return 1
"""
