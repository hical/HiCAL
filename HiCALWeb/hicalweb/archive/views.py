import logging

from braces import views
from django.views import generic
from django.db.models import Q

from hicalweb.judgment.models import Judgement

logger = logging.getLogger(__name__)


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'archive/home.html'

    def get_context_data(self, **kwargs):
        context = super(HomePageView, self).get_context_data(**kwargs)

        judgments = Judgement.objects.filter(Q(user=self.request.user,
                                             task=self.request.user.current_task) &
                                           (
                                               Q(highlyRelevant=True) |
                                               Q(relevant=True) |
                                               Q(nonrelevant=True)
                                           ))
        context["judgments"] = judgments

        return context

    def get(self, request, *args, **kwargs):
        return super(HomePageView, self).get(self, request, *args, **kwargs)
