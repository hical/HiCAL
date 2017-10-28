import logging

from braces import views
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic
from treccoreweb.judgment.models import Judgement

logger = logging.getLogger(__name__)
from django.shortcuts import render


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'archive/home.html'

    def get_context_data(self, **kwargs):
        context = super(HomePageView, self).get_context_data(**kwargs)

        judgments = Judgement.objects.filter(user=self.request.user,
                                             task=self.request.user.current_task)
        context["judgments"] = judgments

        return context

    def get(self, request, *args, **kwargs):
        return super(HomePageView, self).get(self, request, *args, **kwargs)
