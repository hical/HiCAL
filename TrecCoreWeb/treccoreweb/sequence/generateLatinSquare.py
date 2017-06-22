#!/usr/bin/env python
# -*- coding: UTF-8  -*-

"""docstring
"""

__revision__ = '0.1'

import sys,os
import getopt
import numpy as np
import json
import random
import scipy.stats as stats
# import matplotlib.pyplot as plt
# import matplotlib.ticker as mtick
# from matplotlib.backends.backend_pdf import PdfPages
import collections

LatinSquare= np.array([["A1","B2","C3","D4","E5"],["C2","D3","E4","A5","B1"],\
	['E3','A4','B5','C1','D2'],['B4','C5','D1','E2','A3'],['D5','E1','A2','B3','C4']])


topicDict ={}
interfaceDict ={}
for i in range(10):
	np.random.shuffle(LatinSquare) 
	np.random.shuffle(np.transpose(LatinSquare))

	# print LatinSquare
	
	j = 0
	for row in np.asarray(LatinSquare):
		firstList = []
		secondList = []
		for item in row:
			# print item[0]
			firstList.append(ord(item[0])-ord('A')+1)
			secondList.append(i*5 + int(item[1]))
		print firstList
		print secondList
		topicDict[i+j*10] = firstList
		interfaceDict[i+j*10] = secondList
		j += 1

for i in topicDict:
	print i
	print topicDict[i]
	print interfaceDict[i]
