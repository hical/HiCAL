from config.settings.base import AUTH_USER_MODEL as User

from django.contrib.postgres.fields import JSONField
from django.db import models

from hicalweb.progress.models import Task


class Judgment(models.Model):
    class Meta:
        unique_together = ['user', 'doc_id', 'task']
        index_together = ['user', 'doc_id', 'task']

    user = models.ForeignKey(User)
    doc_id = models.CharField(null=False, blank=False, max_length=512)
    task = models.ForeignKey(Task)
    # a judgment can have null fields if its only been viewed but not judged
    # 0 = Non Relevant, 1 = Relevant, 2 = Highly Relevant
    rel = models.IntegerField(null=True, blank=True)
    # set only when an explicit judgment is made
    source = models.CharField(null=True, blank=True, max_length=24)
    # history of active and away time spent on the document
    time_verbose = JSONField(null=True, blank=True, default=list, verbose_name="History")
    meta = JSONField(null=True, blank=True, default=list)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "{} on {}: {}".format(self.user, self.doc_id, self.rel)

    def __str__(self):
        return self.__unicode__()
