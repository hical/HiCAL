from django.db import models
from config.settings.base import AUTH_USER_MODEL as User
import uuid


class Topic(models.Model):
    username = models.ForeignKey(User)

    number = models.PositiveIntegerField(null=True,
                                         blank=True)
    title = models.CharField(null=False,
                             blank=False,
                             max_length=512)
    seed_query = models.CharField(null=False,
                                  blank=False,
                                  max_length=512)
    description = models.TextField(null=True,
                                   blank=True)
    uuid = models.UUIDField(default=uuid.uuid4,
                            editable=False)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "<User:{}, TopicNum:{}>".format(self.username, self.number)

    def __str__(self):
        return self.__unicode__()

