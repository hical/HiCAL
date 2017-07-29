import csv

from django.core.management.base import BaseCommand

from treccoreweb.judgment.models import Judgement


class Command(BaseCommand):
    help = 'Export judgments to csv format'
    filename = 'TRECjudgments.csv'
    header = ('USER', 'TASK', 'TOPIC', 'DOCID')

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Writing to "
                                             "file: {}".format(self.filename)))
        with open(self.filename, 'wt') as f:
            writer = csv.writer(f)
            writer.writerow(self.header)
            judgments = Judgement.objects.order_by('user', 'created_at')
            for judgment in judgments:
                value = 2 if judgment.highlyRelevant else 1 if judgment.relevant else 0 if judgment.nonrelevant else None
                # if value is empty, then user looked at document but not has not judged
                if value is None:
                    continue

                user = judgment.user
                task = judgment.task.setting
                topic = judgment.task.topic.number
                docid = judgment.doc_id

                writer.writerow((user, task, topic, docid))

        self.stdout.write(self.style.SUCCESS(
            'Successfully exported judgments to {}'.format(self.filename)))
