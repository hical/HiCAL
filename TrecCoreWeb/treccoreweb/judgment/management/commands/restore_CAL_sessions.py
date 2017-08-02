from django.core.management.base import BaseCommand

from treccoreweb.judgment.models import Judgement
from treccoreweb.progress.models import Task
from config.settings.base import CAL_SERVER_IP, CAL_SERVER_PORT
import requests


class Command(BaseCommand):
    help = 'Restore CAL sessions'

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Restoring CAL sessions"))

        judgments = {}
        for row in Task.objects.all():
            session_id = str(row.uuid)
            seed_query = str(row.topic.seed_query)

            judgments[(session_id, seed_query)] = []

        for row in Judgement.objects.all():
            session_id = str(row.task.uuid)
            seed_query = str(row.task.topic.seed_query)

            judgments[(session_id, seed_query)].append((row.doc_id, -1 if row.nonrelevant else 1))


        for session_id, seed_query in judgments:
            url = "http://{}:{}/CAL/begin".format(CAL_SERVER_IP, CAL_SERVER_PORT)
            print("Restoring {}: '{}'...".format(session_id, seed_query))
            seed_docs = ','.join([doc_id + ':' + str(rel) for doc_id, rel in judgments[(session_id, seed_query)]])

            data = 'session_id=%s&seed_query=%s&seed_judgments=%s&mode=para' % (session_id, seed_query, seed_docs)
            # print(data)
            resp = requests.post(url, data=data)
          
            # if resp.status != '200':
            #     print("Session {}-'{}' already exists".format(session_id, seed_query))

        self.stdout.write(self.style.SUCCESS(
            'Requests for all sessions are completed.'))
