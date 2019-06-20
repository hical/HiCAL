import logging

from braces import views
from django.views import generic
from django.db.models import Q

from hicalweb.judgment.models import Judgment

logger = logging.getLogger(__name__)


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'archive/home.html'

    def get_context_data(self, **kwargs):
        context = super(HomePageView, self).get_context_data(**kwargs)

        judgments = Judgment.objects.filter(Q(user=self.request.user,
                                             task=self.request.user.current_task) &
                                           (
                                               Q(rel=2) |
                                               Q(rel=1) |
                                               Q(rel=0)
                                           ))
        context["judgments"] = judgments

        return context

    def get(self, request, *args, **kwargs):
        return super(HomePageView, self).get(self, request, *args, **kwargs)
