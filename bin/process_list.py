#!/usr/bin/env python2

import re
from interface_list import interface;

# IUnknown interface
IUnknown = [ "IUnknown", None, "",
	[["HRESULT", "QueryInterface", ["REFIID riid", "void**out"]],
	["ULONG", "AddRef", []], ["ULONG", "Release", []]]]

def argsStr_(args):
	s = ""
	needComma = False
	for x in args:
		if needComma: s += ', '
		needComma = True
		s += str(x)
	return s

def argsStr(type, name, args):
	return '%s %s(%s)' % (type, name, argsStr_(args))

def funcStr(funcs):
	s = ""
	for func in funcs:
		s += '\t%s\n' % str(func)
	return s

def interStr(type, base, funcs):
	return 'interface %s : %s\n{\n%s}\n\n' % (type, base, funcs)

class FuncArg:
	def __init__(self, x):
		x = re.match(r'(.*?)\s*?(\w+)?$', x.strip())
		self.type = x.group(1)
		self.name = x.group(2)

	def cmp(self, that):
		if self.type != that.type: return -1;
		if self.name != that.name: return 0;
		return 1;

	def __str__(self):
		s = self.type
		if self.name:
			s += ' '+self.name
		return s

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

	def nArgs(self):
		return len(self.args)

	def lnStr(self):
		return '%s@%d' % (self.name, self.nArgs())

	def __str__(self):
		return argsStr(self.type, self.name, self.args)

class Interface:
	def __init__(self, x):
		self.type = x[0];
		self.base = x[1];
		self.files = [x[2]]
		self.funcs = []
		self.done = False

		for func in x[3]:
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

	def list_funcs(self, funcList=None):
		if funcList == None: funcList = []
		for func in self.funcs:
			funcList.append(func.name)
		return funcList;


	def str_funcs(self):
		def xxx():
			for x in self.funcs: yield x.lnStr()
		return argsStr_(xxx())

	def __str__(self):
		return interStr(self.type, self.base, funcStr(self.funcs))

	def merge(self, inter):
		diff = self.cmp(inter)
		if diff > 0: self.files += inter.files
		return diff

	def nFuncs(self):
		return len(self.funcs)

class Interface_Group:
	def __init__(self, inter):
		self.inters = [inter]

	def merge(self, inter):
		bestDiff = -1
		for x in self.inters:
			diff = x.merge(inter)
			bestDiff = max(bestDiff, diff)
			if bestDiff > 0: break
		if bestDiff == 0:
			self.inters.append(inter)
		return bestDiff

	# merged Interface properties
	def trim_funcs(self, funcList):
		for inter in self.inters:
			inter.trim_funcs(funcList)
	@property
	def files(self):
		files = []
		for inter in self.inters:
			files += inter.files
		return files

	# common Interface properties
	def list_funcs(self, funcList=None):
		return self.inters[0].list_funcs(funcList)
	def str_funcs(self): return self.inters[0].str_funcs()
	def nFuncs(self): return self.inters[0].nFuncs()
	@property
	def type(self):	return self.inters[0].type
	@property
	def base(self): return self.inters[0].base

class Interface_List:
	def __init__(self, s=None):
		self.groups = []
		if s: self.merge(s)

	def merge(self, s):
		inter = Interface(s)
		for x in self.groups:
			diff = x.merge(inter)
			if diff >= 0: return
		self.groups.append(Interface_Group(inter))

	@property
	def type(self):
		return self.groups[0].type

	def bad(self):
		assert(len(self.groups) != 0)
		return len(self.groups) > 1

	def __str__(self):
		s = 'interface %s {\n' % self.type
		for x in self.groups:
			s += '  %s : %s\n    %s\n' % (x.base, x.str_funcs(), argsStr_(x.files))
		return s + '}\n\n'

# create interface list
interfaceDict = {"IUnknown":Interface_List(IUnknown)}
for s in interface:
	interfaceDict.setdefault(s[0], Interface_List()).merge(s)

for k,v in interfaceDict.iteritems():
	if v.bad(): print v


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
