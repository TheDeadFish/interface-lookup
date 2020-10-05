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

class Function:
	def __init__(self, x):
		self.type = x[0]
		self.name = x[1]
		self.args = x[2]

	def cmp(self, that):
		if self.name != that.name: return -2
		if len(self.args) != len(that.args): return -1
		if self.type != that.type: return 0
		for i in range(len(self.args)):
			if self.args[i] != that.args[i]:
				return 0
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
		retVal = False; i = 0;
		for name in funcList:
			if i >= len(self.funcs): break;
			if self.funcs[i].name == name:
				del self.funcs[i]; retVal = True
			else: i += 1
		return retVal;

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
		retVal = False
		for inter in self.inters:
			retVal = inter.trim_funcs(funcList)
		return retVal

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
		self.state = 0;
		self.groups = []
		if s: self.merge(s)

	def merge(self, s):
		inter = Interface(s)
		for x in self.groups:
			diff = x.merge(inter)
			if diff >= 0: return
		self.groups.append(Interface_Group(inter))

	# common Interface properties
	def trim_funcs(self, funcList):
		return self.groups[0].trim_funcs(funcList)
	def list_funcs(self, funcList=None):
		return self.groups[0].list_funcs(funcList)
	@property
	def type(self): return self.groups[0].type
	@property
	def base(self): return self.groups[0].base

	def bad(self):
		assert(len(self.groups) != 0)
		return len(self.groups) > 1

	def __str__(self):
		s = 'interface %s {\n' % self.type
		for x in self.groups:
			s += '  %s : %s\n    %s\n' % (x.base, x.str_funcs(), argsStr_(x.files))
		return s + '}\n'

	def warn(self, msg):
		print '[%s]\n%s\n' % (msg, str(self))

	def err(self, msg):
		if self.state < 0: return;
		self.state = -1
		self.warn(msg)

	@property
	def inters(self):
		for x in self.groups:
			for y in x.inters:
				yield y

	def process(self, dict):

		if self.type == "IUnknown":
			return self.list_funcs();
		if self.state < 0: return None
		if self.bad():
			self.err('conflict'); return None

		# check base class
		if self.base not in dict:
			self.err('base not found'); return None
		funcList = dict[self.base].process(dict)
		if funcList == None:
			self.err('bad base'); return None

		# combine core
		if self.state == 0:
			self.state = 1
			if self.trim_funcs(funcList):
				self.warn('trimmed')
		return self.list_funcs(funcList);


# create interface list
with open('interface_reject.txt', 'r') as f:
	interfaceReject = [line.rstrip().split(',') for line in f]
interfaceDict = {"IUnknown":Interface_List(IUnknown)}

def match_reject(s):
	for reject in interfaceReject:
		match_file = (not reject[0]) or (reject[0] == s[2])
		match_name = (len(reject)<2) or (reject[1] == s[0])
		if match_file and match_name: return True
	return False

for s in interface:
	if not match_reject(s):
		interfaceDict.setdefault(s[0], Interface_List()).merge(s)

for k,v in interfaceDict.iteritems():
	v.process(interfaceDict)

# produce output
fpOut = open('interface_data.txt', 'w')
for k,v in interfaceDict.iteritems():
	if v.state < 1: continue

	# write type, base, functions
	fpOut.write("#%s,%s" % (v.type, v.base))
	for x in v.list_funcs():
		fpOut.write(",%s" % x)
	fpOut.write("\n")

	# write filenames and arguments
	for inter in v.inters:
		fpOut.write(','.join(inter.files)+'\n')
		for func in inter.funcs:
			fpOut.write('%s,%s' % (func.type, ','.join(func.args)))
			fpOut.write('\n')
