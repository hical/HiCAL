from config.settings.base import AUTH_USER_MODEL as User

from django.contrib.postgres.fields import JSONField
from django.db import models

from hicalweb.progress.models import Session


class Judgment(models.Model):
    class Meta:
        unique_together = ['user', 'doc_id', 'task']
        index_together = ['user', 'doc_id', 'task']

    user = models.ForeignKey(User)
    task = models.ForeignKey(Session)

    doc_id = models.CharField(null=False, blank=False, max_length=512)
    doc_title = models.TextField(null=False, blank=False)
    doc_CAL_snippet = models.TextField(null=False, blank=False)
    doc_search_snippet = models.TextField(null=False, blank=False)
    query = models.TextField(null=True, blank=True)

    # A judgment can have null fields if its only been viewed but not judged
    # 2 indicates highly rel, 1 for rel, and 0 for non-relevant.
    relevance = models.IntegerField(null=True, blank=True)

    # method used to make the judgment: click/keyboard
    method = models.CharField(null=True, blank=True, max_length=64)

    # source of judgment: search/searchModal/CAL
    source = models.CharField(null=True, blank=True, max_length=64)

    # Search query and Ctrl+F terms related fields
    search_query = models.TextField(null=True, blank=True)
    ctrl_f_terms_input = models.TextField(null=True, blank=True)
    found_ctrl_f_terms_in_title = JSONField(null=True, blank=True, default=list)
    found_ctrl_f_terms_in_summary = JSONField(null=True, blank=True, default=list)
    found_ctrl_f_terms_in_full_doc = JSONField(null=True, blank=True, default=list)

    # history of active and away time spent on the document
    timeVerbose = JSONField(null=True, blank=True, default=list, verbose_name="History")

    created_at = models.DateTimeField(auto_now_add=True, editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "{} on {}: {}".format(self.user, self.doc_id, self.relevance)

    def __str__(self):
        return self.__unicode__()
