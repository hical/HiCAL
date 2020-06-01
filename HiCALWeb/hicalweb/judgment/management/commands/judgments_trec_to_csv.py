import csv

from django.core.management.base import BaseCommand

from hicalweb.judgment.models import Judgment


class Command(BaseCommand):
    help = 'Export judgments to csv format'
    filename = 'TRECjudgments.csv'
    header = ('USER', 'TOPIC', 'DOCID', 'JUDGMENT')

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Writing to "
                                             "file: {}".format(self.filename)))
        with open(self.filename, 'wt') as f:
            writer = csv.writer(f)
            writer.writerow(self.header)
            judgments = Judgment.objects.order_by('user', 'created_at')
            for judgment in judgments:
                value = judgment.relevance
                # if value is empty, then user looked at document but not has not judged
                if value is None:
                    continue

                user = judgment.user
                topic = judgment.task.topic.number
                docid = judgment.doc_id

                writer.writerow((user, topic, docid, value))

        self.stdout.write(self.style.SUCCESS(
            'Successfully exported judgments to {}'.format(self.filename)))
