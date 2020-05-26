from django.db import models


class Topic(models.Model):
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
    display_description = models.TextField(null=True,
                                           blank=True)
    narrative = models.TextField(null=True,
                                 blank=True)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "{} {}".format(self.number, self.title)

    def __str__(self):
        return self.__unicode__()
