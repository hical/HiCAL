from config.settings.base import AUTH_USER_MODEL as User
import uuid

from django.db import models

from treccoreweb.CAL.exceptions import CALError
from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.topic.models import Topic


class TaskSetting(models.Model):
    """
    Interface and document settings for tasks

    """

    show_search = models.BooleanField()
    toggle_doc = models.BooleanField()
    only_show_doc = models.BooleanField()

    def __unicode__(self):
        show_search = "T" if self.show_search else "F"
        toggle_doc = "T" if self.toggle_doc else "F"
        only_show_doc = "T" if self.only_show_doc else "F"
        return "{}{}{}".format(show_search, toggle_doc, only_show_doc)

    def __str__(self):
        return self.__unicode__()


class Task(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)
    setting = models.ForeignKey(TaskSetting,
                                default=1)
    uuid = models.UUIDField(default=uuid.uuid4,
                            editable=False)

    # current task active time (in seconds)
    timespent = models.FloatField(default=0)
    # last activity timestamp
    last_activity = models.FloatField(default=None, null=True, blank=True)

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
