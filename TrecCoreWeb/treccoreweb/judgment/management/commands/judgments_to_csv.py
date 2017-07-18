import csv

from django.core.management.base import BaseCommand

from treccoreweb.judgment.models import Judgement


class Command(BaseCommand):
    help = 'Export judgments to csv format'
    filename = 'judgments.csv'
    header = ('USER', 'TASK', 'TOPIC', 'DOCID', 'JUDGMENT', 'TIME_TO_JUDGE')

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Writing to "
                                             "file: {}".format(self.filename)))
        with open(self.filename, 'wt') as f:
            writer = csv.writer(f)
            writer.writerow(self.header)
            judgments = Judgement.objects.all()
            for judgment in judgments:
                value = 2 if judgment.highlyRelevant else 1 if judgment.relevant else 0 if judgment.nonrelevant else None
                # if value is empty, then user looked at document but not has not judged
                if not value:
                    continue
                user = judgment.user
                task = judgment.task.setting
                topic = judgment.task.topic.number
                docid = judgment.doc_id
                time_to_judge = 0.0
                for d in judgment.timeVerbose:
                    # judgments from search SERP don't have a time counter
                    if d.get('source') == 'searchSERP':
                        continue
                    time_to_judge += d.get('timeActive')
                time_to_judge /= 1000
                writer.writerow((user, task, topic, docid, value, time_to_judge))

        self.stdout.write(self.style.SUCCESS(
            'Successfully exported judgments to {}'.format(self.filename)))
