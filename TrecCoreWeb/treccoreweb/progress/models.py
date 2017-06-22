from django.db import models
from model_utils import Choices
from config.settings.base import AUTH_USER_MODEL as User
from treccoreweb.topic.models import Topic
import datetime

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


class Demographic(models.Model):
    AGREEMENT_CHOICES = Choices(
        ("", ""),
        ("STRONG_DISAGREE", "Strongly Disagree"),
        ("DISAGREE", "Disagree"),
        ("NEUTRAL", "Neutral"),
        ("AGREE", "Agree"),
        ("STRONGLY_AGREE", "Strongly Agree")
    )

    username = models.ForeignKey(User)

    age = models.PositiveIntegerField()

    GENDER_CHOICES = Choices(
        ("", ""),
        ("MALE", "Male"),
        ("FEMALE", "Female")
    )
    gender = models.CharField(max_length=56,
                              choices=GENDER_CHOICES)

    DEGREE_CHOICES = Choices(
        ("", ""),
        ("UNDERGRAD", "An undergraduate student"),
        ("GRAD", "A graduate student"),
        ("OTHER", "Other. Please specify"),
    )
    student_degree = models.CharField(max_length=56,
                                      choices=DEGREE_CHOICES)
    student_degree_other = models.CharField(max_length=128,
                                            null=True,
                                            blank=True)

    MAJOR_CHOICES = Choices(
        ("", ""),
        ("ART", "An art student"),
        ("STEM", "A science, technology, engineering, or math student."),
        ("OTHER", "Other. Please specify")
    )
    student_major = models.CharField(max_length=56,
                                     choices=MAJOR_CHOICES)
    student_major_other = models.CharField(max_length=128,
                                           null=True,
                                           blank=True)

    LANGUAGE_FLUENCY_CHOICES = Choices(
        ("", ""),
        ("BEGINNER", "Beginner"),
        ("INTERMEDIATE", "Intermediate"),
        ("ADVANCE", "Advance"),
        ("FLUENT", "Fluent"),
        ("PROFICIENT", "Proficient"),
        ("NATIVE", "Native")
    )
    language = models.CharField(max_length=56,
                                choices=LANGUAGE_FLUENCY_CHOICES)

    SEARCH_ENGINE_USAGE = Choices(
        ("", ""),
        ("SEVERAL", "Several times a day"),
        ("ONCE", "At least once a day"),
        ("WEEK", "At least once a week"),
        ("MONTH", "At least once a month"),
        ("RARELY", "Rarely (Less than one search a month on average)"),
    )
    search_engine_usage = models.CharField(max_length=56,
                                           choices=SEARCH_ENGINE_USAGE)

    EXPERTISE_CHOICES = AGREEMENT_CHOICES
    expertise = models.CharField(max_length=56,
                                 choices=EXPERTISE_CHOICES)

    TROUBLE_CHOICES = AGREEMENT_CHOICES
    trouble = models.CharField(max_length=56,
                               choices=TROUBLE_CHOICES)

    HELP_CHOICES = AGREEMENT_CHOICES
    help = models.CharField(max_length=56,
                            choices=HELP_CHOICES)

    ENJOYMENT_CHOICES = AGREEMENT_CHOICES
    enjoyment = models.CharField(max_length=56,
                                 choices=ENJOYMENT_CHOICES)

    SPECIAL_TRAINING_CHOICES = Choices(
        ("", ""),
        ("YES", "Yes"),
        ("NO", "No"),
    )
    training = models.CharField(max_length=56,
                                choices=SPECIAL_TRAINING_CHOICES)
    training_feedback = models.TextField(null=True,
                                         blank=True)

    feedback = models.TextField(null=True,
                                blank=True)

    def __unicode__(self):
        return "<User:{}>".format(self.username)

    def __str__(self):
        return self.__unicode__()


class PreTask(models.Model):
    username = models.ForeignKey(User)
    task = models.ForeignKey('Task', related_name='pretask_task')

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
        return "<User:{}, Task:{}>".format(self.username, self.task)

    def __str__(self):
        return self.__unicode__()


class PostTask(models.Model):
    username = models.ForeignKey(User)
    task = models.ForeignKey('Task', related_name='posttask_task')

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
        return "<User:{}, Task:{}>".format(self.username, self.task)

    def __str__(self):
        return self.__unicode__()


class TaskSetting(models.Model):
    """
    Interface and document settings for tasks

    """

    show_search = models.BooleanField()
    show_CAL = models.BooleanField()
    show_summary = models.BooleanField()
    show_full_doc = models.BooleanField()


class Task(models.Model):
    username = models.ForeignKey(User)
    topic = models.ForeignKey(Topic)
    pretask = models.ForeignKey(PreTask, related_name='task_pretask')
    posttask = models.ForeignKey(PostTask, related_name='task_posttask')
    setting = models.ForeignKey(TaskSetting)

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
