from config.settings.base import AUTH_USER_MODEL as User
import uuid

from django.db import models

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.topic.models import Topic


class Task(models.Model):
    STRATEGY_CHOICES = (
        ('doc', 'Full document (CAL)'),
        ('para', 'Paragraph (CAL)'),
        ('para_scal', 'Paragraph (S-CAL)'),
    )

    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)
    uuid = models.UUIDField(default=uuid.uuid4,
                            editable=False)

    # max number of judgments you wish for this task. 0 or negative to have no max.
    max_number_of_judgments = models.IntegerField(null=False, blank=False)
    strategy = models.CharField(max_length=64,
                                choices=STRATEGY_CHOICES,
                                null=False,
                                blank=False)
    # For paragraphs strategies
    show_full_document_content = models.BooleanField(null=False,
                                                     blank=False)
    # current task active time (in seconds)
    timespent = models.FloatField(default=0)
    # last activity timestamp
    last_activity = models.FloatField(default=None, null=True, blank=True)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def save(self, *args, **kwargs):
        if not self.pk:
            try:
                CALFunctions.add_session(str(self.uuid),
                                         self.topic.seed_query,
                                         self.strategy)
            except (CALError, ConnectionRefusedError, Exception) as e:
                # TODO: log error
                pass
        super(Task, self).save(*args, **kwargs)

    def __unicode__(self):
        return "<User:{}, Num:{}>".format(self.username, self.topic.number)

    def __str__(self):
        return self.__unicode__()
