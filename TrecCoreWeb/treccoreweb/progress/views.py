from config.settings.base import PRACTICE_PASSWORD
from config.settings.base import PRACTICE_USERNAME
import logging
from django.db.models import Count, Case, When

from allauth.account.adapter import get_adapter
from allauth.account.utils import perform_login
from braces import views
from django.contrib import messages
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic
from treccoreweb.judgment.models import Judgement
from treccoreweb.progress.models import Task

from treccoreweb.progress.forms import TaskForm
from treccoreweb.progress.forms import TopicForm


logger = logging.getLogger(__name__)


class Home(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/home.html'

    def get_context_data(self, **kwargs):
        context = super(Home, self).get_context_data(**kwargs)
        # FORMS
        context['form'] = TaskForm()
        context['form_topic'] = TopicForm()

        # COUNTERS
        counters = Judgement.objects.filter(user=self.request.user,
                                            isFromCAL=True,
                                task=self.request.user.current_task).aggregate(
            total_highlyRelevant=Count(Case(When(highlyRelevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_relevant=Count(Case(When(relevant=True, then=1)))
        )

        context["total_highlyRelevant_CAL"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_CAL"] = counters["total_nonrelevant"]
        context["total_relevant_CAL"] = counters["total_relevant"]

        counters = Judgement.objects.filter(user=self.request.user,
                                            isFromSearch=True,
                                task=self.request.user.current_task).aggregate(
            total_highlyRelevant=Count(Case(When(highlyRelevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_relevant=Count(Case(When(relevant=True, then=1)))
        )

        context["total_highlyRelevant_search"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_search"] = counters["total_nonrelevant"]
        context["total_relevant_search"] = counters["total_relevant"]


        return context

    def get(self, request, *args, **kwargs):
        return super(Home, self).get(self, request, *args, **kwargs)

    def post(self, request, *args, **kwargs):
        success_message = 'Your topic has been initialized. ' \
                          'Choose a platform from the left sidebar to start judging.'
        if 'submit-task-form' in request.POST:
            form = TaskForm(request.POST)
            if form.is_valid():
                f = form.save(commit=False)
                f.username = request.user
                f.save()
                self.request.user.current_task = form.instance
                self.request.user.save()
                messages.add_message(request,
                                     messages.SUCCESS,
                                     success_message)
        elif 'submit-topic-form' in request.POST:
            form = TopicForm(request.POST)
            if form.is_valid():
                f = form.save(commit=False)
                f.save()
                task = Task.objects.create(
                    username=self.request.user,
                    topic=form.instance,
                )
                messages.add_message(request,
                                     messages.SUCCESS,
                                     success_message)
                self.request.user.current_task = task
                self.request.user.save()
        else:
            messages.add_message(request,
                                 messages.DANGER,
                                 'Ops. Something went wrong.')
        return HttpResponseRedirect(reverse_lazy('progress:home'))


class VisitAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
            page_file = self.request_json.get(u"page_file")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u"pag_file and page_title"}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your visit has been recorded"}
        return self.render_json_response(context)


class CtrlFAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            search_field_value = self.request_json.get(u"search_field_value")
            page_title = self.request_json.get(u"page_title")
            extra_context = self.request_json.get(u"extra_context")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, "
                                      u"extra_context and search_field_value"}
            return self.render_bad_request_response(error_dict)


        context = {u"message": u"Your event has been recorded"}
        return self.render_json_response(context)


class FindKeystrokeAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            doc_id = self.request_json.get(u"doc_id")
            page_title = self.request_json.get(u"page_title")
            character = self.request_json.get(u"character")
            isSearchbarFocused = self.request_json.get(u"isSearchbarFocused")
            search_bar_value = self.request_json.get(u"search_bar_value")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u" doc_id, character, isSearchbarFocused,"
                                      u" page_title and search bar value."}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your visit has been recorded."}
        return self.render_json_response(context)


class MessageAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                      views.JsonRequestResponseMixin,
                      generic.View):
    """
    Generic view to capture specific log messages from browser
    """
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            message = self.request_json.get(u"message")
            action = self.request_json.get(u"action")
            page_title = self.request_json.get(u"page_title")
            extra_context = self.request_json.get(u'extra_context')
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, "
                                      u"message, ... etc"}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your log message with action '{}' "
                               u"has been logged.".format(action)}

        return self.render_json_response(context)


class PracticeCompleteView(views.LoginRequiredMixin, generic.TemplateView):

    def get(self, request, *args, **kwargs):
        adapter = get_adapter(self.request)
        adapter.logout(self.request)
        return HttpResponseRedirect(reverse_lazy('account_login'))


class PracticeView(generic.TemplateView):

    def get(self, request, *args, **kwargs):
        credentials = {
            "username": PRACTICE_USERNAME,
            "password": PRACTICE_PASSWORD
        }
        user = get_adapter(self.request).authenticate(
            self.request,
            **credentials)
        ret = perform_login(request, user,
                            email_verification=False,
                            redirect_url=reverse_lazy('progress:home'))
        return ret
