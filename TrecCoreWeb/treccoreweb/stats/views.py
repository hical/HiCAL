from braces import views
from django.db.models import Count, Case, When

from django.views import generic
from treccoreweb.judgment.models import Judgement

import logging
import statistics
logger = logging.getLogger(__name__)


class StatsHomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'stats/home.html'

    def get_context_data(self, **kwargs):
        context = super(StatsHomePageView, self).get_context_data(**kwargs)

        # COUNTERS

        counters = Judgement.objects.filter(user=self.request.user,
                                            isFromCAL=True,
                                topic=self.request.user.current_task.topic).aggregate(
            total_highlyRelevant=Count(Case(When(highlyRelevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_relevant=Count(Case(When(relevant=True, then=1)))
        )

        context["total_highlyRelevant_CAL"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_CAL"] = counters["total_nonrelevant"]
        context["total_relevant_CAL"] = counters["total_relevant"]

        counters = Judgement.objects.filter(user=self.request.user,
                                            isFromSearch=True,
                                topic=self.request.user.current_task.topic).aggregate(
            total_highlyRelevant=Count(Case(When(highlyRelevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_relevant=Count(Case(When(relevant=True, then=1)))
        )

        context["total_highlyRelevant_CAL"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_CAL"] = counters["total_nonrelevant"]
        context["total_relevant_CAL"] = counters["total_relevant"]

        # MEAN, MEDIAN, STDEV, AND VAR

        CALJudgments = Judgement.objects.filter(user=self.request.user,
                                                isFromCAL=True,
                                                relevant=True,
                                                topic=self.request.user.current_task.topic)
        if CALJudgments:
            total = [float(judgment.time_to_judge) for judgment in CALJudgments]
            context["average_highlyRelevant_timespent_CAL"] = round(statistics.mean(total), 3)
            context["median_highlyRelevant_timespent_CAL"] = statistics.median(total)
            if len(CALJudgments) > 1:
                context["stdev_highlyRelevant_timespent_CAL"] = round(statistics.stdev(total), 3)
                context["variance_highlyRelevant_timespent_CAL"] = round(statistics.variance(total), 3)

        CALJudgments = Judgement.objects.filter(user=self.request.user,
                                                isFromCAL=True,
                                                nonrelevant=True,
                                                topic=self.request.user.current_task.topic)

        if CALJudgments:
            total = [float(judgment.time_to_judge) for judgment in CALJudgments]
            context["average_nonrelevant_timespent_CAL"] = round(statistics.mean(total), 3)
            context["median_nonrelevant_timespent_CAL"] = statistics.median(total)
            if len(CALJudgments) > 1:
                context["stdev_nonrelevant_timespent_CAL"] = round(statistics.stdev(total), 3)
                context["variance_nonrelevant_timespent_CAL"] = round(statistics.variance(total), 3)

        CALJudgments = Judgement.objects.filter(user=self.request.user,
                                                isFromCAL=True,
                                                ontopic=True,
                                                topic=self.request.user.current_task.topic)

        if CALJudgments:
            total = [float(judgment.time_to_judge) for judgment in CALJudgments]
            context["average_relevant_timespent_CAL"] = round(statistics.mean(total), 3)
            context["median_relevant_timespent_CAL"] = statistics.median(total)
            if len(CALJudgments) > 1:
                context["stdev_relevant_timespent_CAL"] = round(statistics.stdev(total), 3)
                context["variance_relevant_timespent_CAL"] = round(statistics.variance(total), 3)

        # TIMES

        CALJudgmentsTimes = Judgement.objects.filter(user=self.request.user,
                                                     isFromCAL=True,
                                                     highlyRelevant=True,
                                                     topic=self.request.user.current_task.topic)
        times = [float(judgment.time_to_judge) for judgment in CALJudgmentsTimes]
        context["times_highlyRelevant_CAL"] = times
        context["times_highlyRelevant_CAL_size"] = len(times)

        CALJudgmentsTimes = Judgement.objects.filter(user=self.request.user,
                                                     isFromCAL=True,
                                                     nonrelevant=True,
                                                     topic=self.request.user.current_task.topic)
        times = [float(judgment.time_to_judge) for judgment in CALJudgmentsTimes]
        context["times_nonrelevant_CAL"] = times
        context["times_nonrelevant_CAL_size"] = len(times)

        CALJudgmentsTimes = Judgement.objects.filter(user=self.request.user,
                                                     isFromCAL=True,
                                                     relevant=True,
                                                     topic=self.request.user.current_task.topic)
        times = [float(judgment.time_to_judge) for judgment in CALJudgmentsTimes]
        context["times_relevant_CAL"] = times
        context["times_relevant_CAL_size"] = len(times)

        return context
