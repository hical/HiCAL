from config.settings.base import AUTH_USER_MODEL as User

from django.contrib.postgres.fields import JSONField
from django.db import models

from treccoreweb.progress.models import Task


class Judgement(models.Model):
    class Meta:
        unique_together = ['user', 'doc_id', 'task']
        index_together = ['user', 'doc_id', 'task']

    LOGGING_MESSAGES = {
        "create": "New judgment.",
        "update": "Updated judgment."
    }

    user = models.ForeignKey(User)
    doc_id = models.CharField(null=False,
                              blank=False,
                              max_length=512)
    doc_title = models.CharField(null=False,
                                 blank=False,
                                 max_length=512)
    doc_CAL_snippet = models.TextField(null=False,
                                       blank=False)
    doc_search_snippet = models.TextField(null=False,
                                          blank=False)
    task = models.ForeignKey(Task)
    query = models.CharField(null=True,
                             blank=True,
                             max_length=512)
    # a judgment can have null fields if its only been viewed but not judged
    highlyRelevant = models.NullBooleanField()
    nonrelevant = models.NullBooleanField()
    relevant = models.NullBooleanField()
    # method used to make the judgment
    fromMouse = models.NullBooleanField()
    fromKeyboard = models.NullBooleanField()
    # set only when an explicit judgment is made
    isFromCAL = models.NullBooleanField()
    isFromSearch = models.NullBooleanField()
    isFromSearchModal = models.NullBooleanField()
    isFromIterative = models.NullBooleanField()
    # Search query and Ctrl+F terms related fields
    search_query = models.TextField(null=True, blank=True)
    ctrl_f_terms_input = models.TextField(null=True, blank=True)
    found_ctrl_f_terms_in_title = JSONField(null=True, blank=True, default={})
    found_ctrl_f_terms_in_summary = JSONField(null=True, blank=True, default={})
    found_ctrl_f_terms_in_full_doc = JSONField(null=True, blank=True, default={})
    # history of active and away time spent on the document
    timeVerbose = JSONField(null=True, blank=True, default=[], verbose_name="History")

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        judgment = 2 if self.highlyRelevant else 0 if self.nonrelevant else 1 if self.relevant else None
        return "{} on {}: {}".format(self.user, self.doc_id, judgment)

    def __str__(self):
        return self.__unicode__()
