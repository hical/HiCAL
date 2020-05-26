import codecs
import re

from bs4 import BeautifulSoup
from django.core.management.base import BaseCommand

from hicalweb.topic.models import Topic

topic_path = "../../fixtures/topics.txt"


class Command(BaseCommand):
    """
    Imports pre-defined topics into database.

    Topics needs to be formatted in TREC XML format. For example:

    <topics>
        <topic>
            <number>3</number>
            <query>acupuncture epilepsy</query>
            <description>
            Can acupuncture be effective for people with epilepsy?
            </description>
            <narrative>
            Acupuncture, a traditional Chinese treatment which is applied by inserting
            thin needles in certain locations of body, has been used as a treatment for
            epilepsy. There are reports that it reduces the regularity and severity of
            epileptic episodes (seizures). A relevant document should discuss whether
            acupuncture can be used to treat epilepsy or control seizures and epilepsy
            symptoms.
            </narrative>
        </topic>
    </topics>

    """
    help = 'Import topics in standard TREC XML format'

    def handle(self, *args, **option):
        self.stdout.write(self.style.SUCCESS("Importing topics..."))

        soup = BeautifulSoup(codecs.open(topic_path, "r"), "lxml")
        exists = 0
        imported = 0
        for topic in soup.findAll("topic"):
            topic_id = topic.findNext("number").getText().strip()
            topic_id = re.sub("[^0-9]", "", topic_id)
            topic_id = int(topic_id) if topic_id else None
            query = topic.findNext("query").getText().strip()
            description = topic.findNext("description").getText().strip()
            narrative = topic.findNext("narrative").getText().strip()

            if not Topic.objects.filter(number=topic_id, title=query).exists():
                Topic.objects.create(
                    number=topic_id,
                    title=query.capitalize(),
                    description=description,
                    narrative=narrative,
                    seed_query="{} {}".format(query, description)
                )
                imported += 1
            else:
                exists += 1

        self.stdout.write(
            self.style.SUCCESS(
                "Imported {} topics. {} topics already exist.".format(imported, exists)
            )
        )
