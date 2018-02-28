from api import delete_session
from api import begin_session
from api import get_docs
from api import judge

def loadQrels(file):
    relDict = {}
    nonrelDict = {}
    allDict = {}
    with open(file,'r') as f:
        for line in f:
            topic, dump, doc, label = line.strip().split()
            if topic in allDict:
                allDict[topic].append(doc)
            else:
                allDict[topic] = [doc]
            if int(label) >= 1:
                if topic in relDict:
                    relDict[topic].append(doc)
                else:
                    relDict[topic] = [doc]
            else:
                if topic in nonrelDict:
                    nonrelDict[topic].append(doc)
                else:
                    nonrelDict[topic] = [doc]

    return relDict, nonrelDict, allDict



def loadRRFRanks(file):
    topicList = []
    searchDocsDict = {}
    with open(file) as rrfF:
        for line in rrfF:
            topic,doc,score, rank = line.strip().split()
            topicList.append(topic)
            if len(doc) < 7:
                doc = '0'+doc
            if topic not in searchDocsDict:
                searchDocsDict[topic] = [doc]
            else:
                searchDocsDict[topic].append(doc)
    return set(topicList), searchDocsDict

def loadSeedQuery(file):
    seedQueryDict = {}
    with open(file) as sF:
        for line in sF:
            topic = line.split()[0]
            seedQuery = line.split(' ', 2)[2]
            seedQueryDict[topic] = seedQuery.strip()

    return seedQueryDict

def simulateCALP():

    topics, searchDocsDict = loadRRFRanks('autonofdbk.rrf.list') 
    seedQueryDict = loadSeedQuery('topics.6')
    relDict, nonrelDict, allDict = loadQrels('qrels.nist.core')

    for topic in topics:
        delete_session(topic)
        # print(seedQueryDict[topic])
        begin_session(topic, seedQueryDict[topic])
        
        iteration = 0
        while iteration < 200:
            docsList = get_docs(topic)
            for doc in docsList:
                if doc in relDict[topic]:
                    judge(topic, doc, 1)
                    print(topic+'\t'+doc+'\t'+str(iteration)+'\t1'+'\tCAL-P')
                else:
                    judge(topic, doc, 0)
                    print(topic+'\t'+doc+'\t'+str(iteration)+'\t0'+'\tCAL-P')
            iteration += 1


if __name__=="__main__":
    simulateCALP()