import os
import django
os.environ["DJANGO_SETTINGS_MODULE"] = 'config.settings.local'
django.setup()

from treccoreweb.judgment.models import Judgement

import requests
API = 'http://localhost:9001/CAL'

judgments = {}
for row in Judgement.objects.all():
    session_id = str(row.topic.uuid)
    seed_query = str(row.topic.seed_query)
    if (session_id,seed_query) not in judgments:
        judgments[(session_id, seed_query)] = []
    judgments[(session_id, seed_query)].append((row.doc_id, -1 if row.nonrelevant else 1))

for session_id, seed_query in judgments:
    resp = requests.post(API+'/begin', data={'session_id': session_id, 'seed_query': seed_query}).json()
    if resp.get('session-id', False) != False:
        print("Restoring", session_id, seed_query)
        for doc_id, rel in judgments[(session_id, seed_query)]:
            requests.post(API+'/judge', data={'session_id': session_id, 'doc_id': doc_id, 'rel': rel})
    else:
        print("Session", session_id, seed_query, "already present")
