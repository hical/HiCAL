from django.contrib.auth.models import AbstractUser
from django.contrib.postgres.fields import JSONField
from django.core.urlresolvers import reverse
from django.db import models
from django.utils.encoding import python_2_unicode_compatible
from django.utils.translation import ugettext_lazy as _

from treccoreweb.progress.models import Task


@python_2_unicode_compatible
class User(AbstractUser):

    # First Name and Last Name do not cover name patterns
    # around the globe.
    name = models.CharField(_('Name of User'), blank=True, max_length=255)
    current_task = models.ForeignKey(Task, blank=True, null=True)
    # sequence of topics assigned to user.
    sequence = JSONField(blank=True, null=True, default=list)
    # treatment number
    treatment = models.IntegerField(blank=True, null=True,
                                    choices=((x, x) for x in range(0, 50)))
    # current task active time (in seconds)
    _cur_task_active_time = models.FloatField(default=0.0)
    # last activity timestamp
    cur_task_last_activity = models.FloatField(default=None, null=True, blank=True)

    # last activity

    def __str__(self):
        return self.username

    def get_absolute_url(self):
        return reverse('users:detail', kwargs={'username': self.username})
