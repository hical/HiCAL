from django.db import models
from model_utils import Choices
from config.settings.base import AUTH_USER_MODEL as User
import uuid
import datetime


class Topic(models.Model):
    number = models.PositiveIntegerField(null=True,
                                         blank=True)
    title = models.CharField(null=False,
                             blank=False,
                             max_length=512)
    seed_query = models.CharField(null=False,
                                  blank=False,
                                  max_length=512)
    description = models.TextField(null=False,
                                   blank=False)
    narrative = models.TextField(null=False,
                                 blank=False)
    uuid = models.UUIDField(default=uuid.uuid4,
                            editable=False)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "<TopicNum:{}>".format(self.number)

    def __str__(self):
        return self.__unicode__()


NA = "---"
NAA = "NOT AT ALL"
SLT = "SLIGHTLY"
SMT = "SOMEWHAT"
MDT = "MODERATELY"
EXT = "EXTREMELY"
LIKERT_SCALE_CHOICES = Choices((NA, "---"),
                               (NAA, "Not at all"),
                               (SLT, "Slightly"),
                               (SMT, "Somewhat"),
                               (MDT, "Moderately"),
                               (EXT, "Extremely")
                               )


class PreTask(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)

    # Pre-task questions
    familiarity = models.CharField(max_length=35,
                                   choices=LIKERT_SCALE_CHOICES,
                                   default=NA)
    difficulty = models.CharField(max_length=35,
                                  choices=LIKERT_SCALE_CHOICES,
                                  default=NA)
    feedback = models.TextField(null=True,
                                blank=True)

    is_completed = models.BooleanField(default=False)
    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "<User:{}, TopicNum:{}>".format(self.username, self.topic.number)

    def __str__(self):
        return self.__unicode__()


class PostTask(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)

    # Post-task questions
    difficulty = models.CharField(max_length=35,
                                  choices=LIKERT_SCALE_CHOICES,
                                  default=NA)
    confidence = models.CharField(max_length=35,
                                  choices=LIKERT_SCALE_CHOICES,
                                  default=NA)
    mood = models.CharField(max_length=35,
                            choices=LIKERT_SCALE_CHOICES,
                            default=NA)
    experience = models.CharField(max_length=35,
                                  choices=LIKERT_SCALE_CHOICES,
                                  default=NA)
    feedback = models.TextField(null=True,
                                blank=True)

    is_completed = models.BooleanField(default=False)
    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "<User:{}, TopicNum:{}>".format(self.username, self.topic.number)

    def __str__(self):
        return self.__unicode__()


class Task(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)
    pretask = models.ForeignKey(PreTask)
    posttask = models.ForeignKey(PostTask)
    # current time spent on task
    timespent = models.DurationField(default=datetime.timedelta)

    def is_time_past(self):
        """
        Check if the task max time been reached.
        :return: True if task max time has been reched.
        """
        return self.timespent >= datetime.timedelta(days=0, hours=1)

    def is_completed(self):
        """
        Check if the task has been completed.
        :return: True if task is completed.
        """
        return self.pretask.is_completed and self.posttask.is_completed

    def is_first_task(self):
        return 1 == self.current_task_number()

    def is_last_task(self):
        return len(self.username.sequence) == self.current_task_number()

    def current_task_number(self):
        current_topic_num = self.topic.number
        ith = self.username.sequence.index(current_topic_num) + 1
        return ith

    def prev_task_number(self):
        """
        E.g. if user currently completing the second task (e.g. second topic),
        return 1.
        :return: the number of the prev task.
        """
        current_topic_num = self.topic.number
        task_num = self.username.sequence.index(current_topic_num)
        if self.username.sequence[0] == self.username.sequence[task_num]:
            return None
        else:
            return task_num - 1

    def next_task_number(self):
        """
        E.g. if user currently completing the first task (e.g. first topic),
        return 2.
        :return: the number of the next task.
        """
        current_topic_num = self.topic.number
        task_num = self.username.sequence.index(current_topic_num)
        if self.username.sequence[-1] == self.username.sequence[task_num]:
            return None
        else:
            return task_num + 2

    def next_task(self):
        """
        E.g. if user currently completing the first task (e.g. first topic),
        return the next (2nd) Task instance.
        :return: Task instance
        """
        next_task_number = self.next_task_number()
        if next_task_number:
            return Task.objects.get(
                username=self.username,
                topic__number=self.username.sequence[next_task_number - 1]
            )
        else:
            return None

    def __unicode__(self):
        return "<User:{}, Num:{}>".format(self.username, self.topic.number)

    def __str__(self):
        return self.__unicode__()
