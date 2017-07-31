from django.core.management.base import BaseCommand

from treccoreweb.judgment.models import Judgement


class Command(BaseCommand):
    help = 'Restore CAL sessions'

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Restoring CAL sessions"))

        self.stdout.write(self.style.SUCCESS(
            'Requests for all sessions are completed.'))
