from config.settings.base import AUTH_USER_MODEL as User
from config.settings.base import MAX_ACTIVE_TIME
import uuid

from django.db import models
from interfaces.Iterative import functions as IterativeEngine
from model_utils import Choices

from treccoreweb.CAL.exceptions import CALError
from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.topic.models import Topic

NA = "---"
VEN = "Very_Not"
SMN = "Somehow_Not"
NNN = "Neither_Nor_Not"
SMT = "Somewhat"
VEY = "Very"

ONE = "1"
TWO = "2"
THREE = "3"
FOUR = "4"
FIVE = "5"

LIKERT_SCALE_CHOICES = Choices((NA, NA),
                               (VEN, "Very Not"),
                               (SMN, "Somehow Not"),
                               (NNN, "Neither Nor Not"),
                               (SMT, "Somewhat"),
                               (VEY, "Very")
                               )
FAM_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                   (VEN, "Very Unfamiliar"),
                                   (SMN, "Somehow Unfamiliar"),
                                   (NNN, "Neutral"),
                                   (SMT, "Somewhat Familiar"),
                                   (VEY, "Very Familiar")
                                   )

DIFF_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                    (VEN, "Very Easy"),
                                    (SMN, "Somehow Easy"),
                                    (NNN, "Neutral"),
                                    (SMT, "Somewhat Hard"),
                                    (VEY, "Very Hard")
                                    )

HELP_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                    (VEN, "Very Useless"),
                                    (SMN, "Somehow Useless"),
                                    (NNN, "Neutral"),
                                    (SMT, "Somewhat Useful"),
                                    (VEY, "Very Useful")
                                    )

CLOSE_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                     (VEN, "Very Far (still lots of relevant documents to be found)"),
                                     (SMN, "Somehow Far"),
                                     (NNN, "Neutral"),
                                     (SMT, "Somewhat Close"),
                                     (VEY, "Very Close (found almost all relevant)")
                                     )

LEFTDOC_SCALE_CHOICES = Choices((NA, NA),
                                ("0", "0"),
                                ("1-49", "1-49"),
                                ("50-99", "50-99"),
                                ("100-499", "100-499"),
                                ("500-999", "500-999"),
                                ("1,000 or more", "1,000 or more")
                                )

INTERFACE_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                         (ONE, "Search Interface & Learning Interface showing "
                                               "Paragraph with Toggle Document"
                                          ),
                                         (TWO,  "Learing Interface showing Paragraph "
                                                "with Toggle Document"),
                                         (THREE, "Search Interface & "
                                                 " Learning Interface showing Paragraph"),
                                         (FOUR, "Learning Interface showing Paragraph"),
                                         (FIVE, "Document Iterative Interface Only")
                                         )

FEAT_LIKERT_SCALE_CHOICES = Choices((NA, NA),
                                    (ONE, "Keywords Search and Highlighting"),
                                    (TWO, "View Full Document"),
                                    (THREE, "Search Interface"),
                                    (FOUR, "Paragraph Interface"),
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

    age = models.PositiveIntegerField(null=True,
                                      blank=True)

    GENDER_CHOICES = Choices(
        ("", ""),
        ("MALE", "Male"),
        ("FEMALE", "Female"),
        ("TERM", "I prefer another term"),
        ("NA", "I prefer not to say")

    )
    gender = models.CharField(max_length=56,
                              choices=GENDER_CHOICES,
                              null=True,
                              blank=True)

    DEGREE_CHOICES = Choices(
        ("", ""),
        ("UNDERGRAD", "An undergraduate student"),
        ("GRAD", "A graduate student"),
        ("OTHER", "Other. Please specify"),
    )
    student_degree = models.CharField(max_length=56,
                                      choices=DEGREE_CHOICES,
                                      null=True,
                                      blank=True)
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
                                     choices=MAJOR_CHOICES,
                                     null=True,
                                     blank=True)
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
                                choices=LANGUAGE_FLUENCY_CHOICES,
                                null=True,
                                blank=True)

    SEARCH_ENGINE_USAGE = Choices(
        ("", ""),
        ("SEVERAL", "Several times a day"),
        ("ONCE", "At least once a day"),
        ("WEEK", "At least once a week"),
        ("MONTH", "At least once a month"),
        ("RARELY", "Rarely (Less than one search a month on average)"),
    )
    search_engine_usage = models.CharField(max_length=56,
                                           choices=SEARCH_ENGINE_USAGE,
                                           null=True,
                                           blank=True)

    EXPERTISE_CHOICES = AGREEMENT_CHOICES
    expertise = models.CharField(max_length=56,
                                 choices=EXPERTISE_CHOICES,
                                 null=True,
                                 blank=True)

    TROUBLE_CHOICES = AGREEMENT_CHOICES
    trouble = models.CharField(max_length=56,
                               choices=TROUBLE_CHOICES,
                               null=True,
                               blank=True)

    HELP_CHOICES = AGREEMENT_CHOICES
    help = models.CharField(max_length=56,
                            choices=HELP_CHOICES,
                            null=True,
                            blank=True)

    ENJOYMENT_CHOICES = AGREEMENT_CHOICES
    enjoyment = models.CharField(max_length=56,
                                 choices=ENJOYMENT_CHOICES,
                                 null=True,
                                 blank=True)

    SPECIAL_TRAINING_CHOICES = Choices(
        ("", ""),
        ("YES", "Yes"),
        ("NO", "No"),
    )
    training = models.CharField(max_length=56,
                                choices=SPECIAL_TRAINING_CHOICES,
                                null=True,
                                blank=True)
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
    task = models.ForeignKey('Task', related_name='pretask_task', null=True, blank=True)

    # Pre-task questions
    familiarity = models.CharField(max_length=35,
                                   choices=FAM_LIKERT_SCALE_CHOICES,
                                   default=NA)
    difficulty = models.CharField(max_length=35,
                                  choices=DIFF_LIKERT_SCALE_CHOICES,
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
    task = models.ForeignKey('Task', related_name='posttask_task', null=True, blank=True)

    # Post-task questions
    difficulty = models.CharField(max_length=35,
                                  choices=DIFF_LIKERT_SCALE_CHOICES,
                                  default=NA)
    close = models.CharField(max_length=35,
                             choices=CLOSE_LIKERT_SCALE_CHOICES,
                             default=NA)
    complete = models.CharField(max_length=35,
                                choices=LEFTDOC_SCALE_CHOICES,
                                default=NA)
    familiarity = models.CharField(max_length=35,
                                   choices=FAM_LIKERT_SCALE_CHOICES,
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


class ExitTask(models.Model):
    username = models.ForeignKey(User)
    difficulty = models.CharField(max_length=35,
                                  choices=INTERFACE_LIKERT_SCALE_CHOICES,
                                  default=NA)
    keyword_search_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)

    keyword_shortcut_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    doc_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    topic_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    recent_doc_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    full_doc_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    quote_helpful = models.CharField(max_length=35,
                               choices=HELP_LIKERT_SCALE_CHOICES,
                               default=NA)
    feedback = models.TextField(null=True,
                                blank=True)

    created_at = models.DateTimeField(auto_now_add=True,
                                      editable=False)
    updated_at = models.DateTimeField(auto_now=True)

    def __unicode__(self):
        return "<User:{}>".format(self.username)

    def __str__(self):
        return self.__unicode__()


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
    pretask = models.ForeignKey(PreTask, related_name='task_pretask')
    posttask = models.ForeignKey(PostTask, related_name='task_posttask')
    setting = models.ForeignKey(TaskSetting)
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
            except CALError as e:
                # TODO: log error
                pass

        super(Task, self).save(*args, **kwargs)

    def is_iterative(self):
        """
        Checks if task is an iterative task (fifth treatment)
        :return: True if it's an iterative task
        """
        return self.setting.only_show_doc

    def is_iterative_completed(self):
        """
        Check if in iterative mode and completed all documents
        :return: True if completed
        """
        iterative_completed_check = False
        # if user in iterative mode
        if self.is_iterative():
            # keep import here to avoid circular imports issues
            from treccoreweb.judgment import helpers
            docs_ids = helpers.remove_judged_docs(IterativeEngine.get_documents(
                                                  self.topic.number),
                                                  self.username,
                                                  self)

            # if there's no more un-judged docs
            if not docs_ids:
                iterative_completed_check = True

        return iterative_completed_check

    def is_time_past(self):
        """
        Check if the task max time been reached.
        :return: True if task max time has been reached.
        """
        return self.timespent >= MAX_ACTIVE_TIME

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