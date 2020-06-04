from django.core.management.base import BaseCommand

from hicalweb.judgment.models import Judgment
from hicalweb.progress.models import Session
from config.settings.base import CAL_SERVER_IP, CAL_SERVER_PORT
import time
import requests


class Command(BaseCommand):
    help = 'Restore CAL sessions'

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Restoring CAL sessions"))
        url = "http://{}:{}/CAL/begin".format(CAL_SERVER_IP, CAL_SERVER_PORT)

        judgments = {}
        for row in Session.objects.all():
            session_id = str(row.uuid)
            seed_query = str(row.topic.seed_query)
            session_strategy = str(row.strategy)

            judgments[(session_id, seed_query, session_strategy)] = []

        for row in Judgment.objects.all():
            session_id = str(row.task.uuid)
            seed_query = str(row.task.topic.seed_query)
            session_strategy = str(row.strategy)

            judgments[(session_id, seed_query, session_strategy)].append((row.doc_id, -1 if row.relevance <= 0 else 1))

        while True:
            print("Waiting for cal server to come online")
            try:
                requests.get(url, timeout=1)
                break
            except requests.Timeout:
                time.sleep(5)

        for session_id, seed_query, session_strategy in judgments:
            print("Restoring {}: '{}'...".format(session_id, seed_query))
            seed_docs = ','.join([doc_id + ':' + str(rel) for doc_id, rel in judgments[(session_id, seed_query)]])

            data = 'session_id={}&seed_query={}&seed_judgments={}&mode={}'.format(
                session_id, seed_query, seed_docs, session_strategy)
            # print(data)
            resp = requests.post(url, data=data)

            # if resp.status != '200':
            #     print("Session {}-'{}' already exists".format(session_id, seed_query))

        self.stdout.write(self.style.SUCCESS(
            'Requests for all sessions are completed.'))
