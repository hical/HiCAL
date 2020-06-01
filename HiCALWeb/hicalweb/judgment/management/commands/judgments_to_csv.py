import csv

from django.core.management.base import BaseCommand

from hicalweb.judgment.models import Judgment


class Command(BaseCommand):
    help = 'Export judgments to csv format'
    filename = 'judgments.csv'
    header = ('USER', 'TOPIC', 'DOCID', 'METHOD', 'JUDGMENT', 'TIME_AWAY',
              'TIME_TO_JUDGE')

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
                method = judgment.source

                time_to_judge = 0.0
                time_away = 0.0
                for d in judgment.timeVerbose:
                    # judgments from search SERP don't have a time counter, set to 0.0
                    if d.get('source') == 'searchSERP':
                        continue
                    time_to_judge += d.get('timeActive')
                    time_away += d.get('timeAway')
                time_away /= 1000
                time_to_judge /= 1000
                writer.writerow((user, topic, docid, method,
                                 value, time_away, time_to_judge))

        self.stdout.write(self.style.SUCCESS(
            'Successfully exported judgments to {}'.format(self.filename)))
