from config.settings.base import AUTH_USER_MODEL as User
import uuid

from django.db import models

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.topic.models import Topic


class Task(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)
    uuid = models.UUIDField(default=uuid.uuid4,
                            editable=False)

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
                CALFunctions.add_session(str(self.uuid), self.topic.seed_query)
            except (CALError, ConnectionRefusedError, Exception) as e:
                # TODO: log error
                pass
        super(Task, self).save(*args, **kwargs)

    def __unicode__(self):
        return "<User:{}, Num:{}>".format(self.username, self.topic.number)

    def __str__(self):
        return self.__unicode__()
