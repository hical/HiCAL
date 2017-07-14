#!/usr/bin/env python
# -*- coding: UTF-8  -*-

"""docstring
"""

__revision__ = '0.1'

import json

import numpy as np

LatinSquare = np.array([["A1", "B2", "C3", "D4", "E5"], ["C2", "D3", "E4", "A5", "B1"],
                        ['E3', 'A4', 'B5', 'C1', 'D2'], ['B4', 'C5', 'D1', 'E2', 'A3'],
                        ['D5', 'E1', 'A2', 'B3', 'C4']])

with open("nist.topics") as f:
    nistTopics = f.readlines()
# you may also want to remove whitespace characters like `\n` at the end of each line
nistTopicList = [int(x.strip()) for x in nistTopics]

# print nistTopicList

np.random.seed(1234)

topicDict = {}
interfaceDict = {}
for i in range(10):
    np.random.shuffle(LatinSquare)
    np.random.shuffle(np.transpose(LatinSquare))

    # print LatinSquare

    j = 0
    for row in np.asarray(LatinSquare):
        seqList = []
        topicList = []
        for item in row:
            # print item[0]
            seqList.append(ord(item[0]) - ord('A') + 1)
            topicList.append(i * 5 + int(item[1]))
        # print firstList
        # print secondList
        interfaceDict[i + j * 10] = seqList
        topicDict[i + j * 10] = topicList
        j += 1

treatmentDict = {1: [True, True, False, True],
                 2: [True, False, False, True],
                 3: [False, True, False, True],
                 4: [False, False, False, True],
                 5: [False, False, True, True]}

treatmentDumps = []
for i in range(0, 50):
    # print i
    aList = topicDict[i]
    # print aList
    topicList = []
    for t in aList:
        topicList.append(nistTopicList[t - 1])
    # print topicList

    bList = interfaceDict[i]
    # print bList
    interfaceList = []
    for interface in bList:
        interfaceList.append(treatmentDict[interface])

    # print interfaceList
    treatment = {"user_ID": i,
                 "treatments": [
                     dict(topic_num=topic, setting=dict(show_search=interface[0],
                                                        toggle_doc=interface[1],
                                                        only_show_doc=interface[2])
                          )
                     for topic, interface in zip(topicList, interfaceList)
                     ]
                 }

    treatmentDumps.append(treatment)

print(json.dumps(treatmentDumps, indent=4))
