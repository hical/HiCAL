import csv

from django.core.management.base import BaseCommand

from hicalweb.judgment.models import Judgment


class Command(BaseCommand):
    help = 'Export judgments to csv format'
    filename = 'judgments.csv'
    header = ('USER', 'TOPIC', 'DOCID', 'SOUCE', 'REL', 'TIME_AWAY', 'TIME_TO_JUDGE')

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Writing to "
                                             "file: {}".format(self.filename)))
        with open(self.filename, 'wt') as f:
            writer = csv.writer(f)
            writer.writerow(self.header)
            judgments = Judgment.objects.order_by('user', 'created_at')
            for judgment in judgments:
                # if rel is empty, then user looked at document but not has not judged
                if judgment.rel is None:
                    continue

                user = judgment.user
                topic = judgment.task.topic.number
                docid = judgment.doc_id
                source = judgment.source

                time_to_judge = 0.0
                time_away = 0.0
                for d in judgment.time_verbose:
                    # judgments from search SERP don't have a time counter, set to 0.0
                    if d.get('source') == 'searchSERP':
                        continue
                    time_to_judge += d.get('timeActive')
                    time_away += d.get('timeAway')
                time_away /= 1000
                time_to_judge /= 1000
                writer.writerow((user, topic, docid, source,
                                 rel, time_away, time_to_judge))

        self.stdout.write(self.style.SUCCESS(
            'Successfully exported judgments to {}'.format(self.filename)))
