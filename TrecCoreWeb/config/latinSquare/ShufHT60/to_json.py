import os

files = [f for f in os.listdir('.') if os.path.isfile(f)]
d = {}
for f in files:
    try:
        topic = int(f[:3])
    except:
        continue
    topic_docs = []
    with open(f, 'r') as l:
        for line in l:
            topic_docs.append(line.strip())
    d[int(topic)] = topic_docs

print(d)
