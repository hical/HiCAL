import json
import logging
import random
import string
import pytz

from allauth.account.adapter import get_adapter
from allauth.account.utils import perform_login
from braces import views
from django.contrib import messages
from django.contrib.auth import get_user_model
from django.contrib.auth.models import Group
from django.db.models import Case
from django.db.models import Count
from django.db.models import When
from django.http import HttpResponseRedirect, HttpResponse, JsonResponse
from django.urls import reverse_lazy
from django.utils import timezone
from django.views import generic

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.judgment.models import Judgment
from hicalweb.progress.forms import SessionPredefinedTopicForm
from hicalweb.progress.forms import SessionForm
from hicalweb.progress.models import Session

logger = logging.getLogger(__name__)


class Home(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/home.html'

    def get_context_data(self, **kwargs):
        context = super(Home, self).get_context_data(**kwargs)
        # FORMS
        context['form'] = SessionPredefinedTopicForm()
        context['form_topic'] = SessionForm()

        # COUNTERS
        counters = Judgment.objects.filter(user=self.request.user,
                                            source="CAL",
                                            task=self.request.user.current_task).aggregate(
            total_highlyRelevant=Count(Case(When(relevance=2, then=1))),
            total_relevant=Count(Case(When(relevance=1, then=1))),
            total_nonrelevant=Count(Case(When(relevance=0, then=1)))
        )

        context["total_highlyRelevant_CAL"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_CAL"] = counters["total_nonrelevant"]
        context["total_relevant_CAL"] = counters["total_relevant"]

        counters = Judgment.objects.filter(user=self.request.user,
                                            source__contains="search",
                                            task=self.request.user.current_task).aggregate(
            total_highlyRelevant=Count(Case(When(relevance=2, then=1))),
            total_relevant=Count(Case(When(relevance=1, then=1))),
            total_nonrelevant=Count(Case(When(relevance=0, then=1)))
        )

        context["total_highlyRelevant_search"] = counters["total_highlyRelevant"]
        context["total_nonrelevant_search"] = counters["total_nonrelevant"]
        context["total_relevant_search"] = counters["total_relevant"]

        return context

    def get(self, request, *args, **kwargs):
        return super(Home, self).get(self, request, *args, **kwargs)

    def post(self, request, *args, **kwargs):
        success_message = 'Your topic has been initialized. ' \
                          'Choose a retrieval method to start judging.'
        if 'submit-session-predefine-topic-form' in request.POST:
            form = SessionPredefinedTopicForm(request.POST)
            if form.is_valid():
                f = form.save(commit=False)
                f.username = request.user
                f.save()
                self.request.user.current_task = form.instance
                self.request.user.save()
                messages.add_message(request,
                                     messages.SUCCESS,
                                     success_message)
        elif 'submit-session-form' in request.POST:
            form = SessionForm(request.POST)
            if form.is_valid():

                f = form.save(commit=False)
                f.save()
                max_number_of_judgments = form.cleaned_data['max_number_of_judgments']
                strategy = form.cleaned_data['strategy']
                show_full_document_content = form.cleaned_data['show_full_document_content']
                task = Session.objects.create(
                    username=self.request.user,
                    topic=form.instance,
                    max_number_of_judgments=max_number_of_judgments,
                    strategy=strategy,
                    show_full_document_content=show_full_document_content
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


class SessionListView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/sessions.html'

    def get_context_data(self, **kwargs):
        context = super(SessionListView, self).get_context_data(**kwargs)
        user_tasks = Session.objects.filter(username=self.request.user).order_by(
            "created_at")

        tasks = []
        for t in user_tasks:
            task_info = {"task_obj": t,
                         "created_at": timezone.localtime(t.created_at,pytz.timezone(
                                                             'America/Toronto'))}

            counters = Judgment.objects.filter(user=self.request.user,
                                                task=t).aggregate(
                total_highlyRelevant=Count(Case(When(relevance=2, then=1))),
                total_relevant=Count(Case(When(relevance=1, then=1))),
                total_nonrelevant=Count(Case(When(relevance=0, then=1)))
            )

            task_info["total_highlyRelevant"] = counters["total_highlyRelevant"]
            task_info["total_nonrelevant"] = counters["total_nonrelevant"]
            task_info["total_relevant"] = counters["total_relevant"]
            task_info["total_judged"] = counters["total_highlyRelevant"] + counters["total_nonrelevant"] + counters["total_relevant"]

            tasks.append(task_info)

        context["tasks"] = tasks

        return context

    def get(self, request, *args, **kwargs):
        return super(SessionListView, self).get(self, request, *args, **kwargs)

    def post(self, request, *args, **kwargs):

        if request.POST.get("activate_sessionid"):
            session_id = request.POST.get("activate_sessionid")

            try:
                task = Session.objects.get(username=self.request.user,
                                           uuid=session_id)
            except Session.DoesNotExist:
                message = 'Ops! your session cant be found.'
                messages.add_message(request,
                                     messages.ERROR,
                                     message)

                return HttpResponseRedirect(reverse_lazy('progress:sessions'))

            self.request.user.current_task = task
            self.request.user.save()

            message = 'Your session has been activated. ' \
                      'Choose a retrieval method to start judging.'
            messages.add_message(request,
                                 messages.SUCCESS,
                                 message)

        elif request.POST.get("delete_sessionid"):
            session_id = request.POST.get("delete_sessionid")
            task = Session.objects.filter(username=self.request.user,
                                          uuid=session_id)
            if task.exists():

                if self.request.user.current_task and str(self.request.user.current_task.uuid) == session_id:
                    self.request.user.current_task = None
                    self.request.user.save()

                task = task.first()
                task.delete()
                try:
                    CALFunctions.delete_session(session_id)
                except CALError:
                    pass
                message = 'Session has been deleted.'
                messages.add_message(request,
                                     messages.SUCCESS,
                                     message)

            else:
                message = 'Ops! session can not be found.'
                messages.add_message(request,
                                     messages.ERROR,
                                     message)

        return HttpResponseRedirect(reverse_lazy('progress:sessions'))


class SessionDetailsAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                             views.JsonRequestResponseMixin,
                             views.AjaxResponseMixin, generic.View):

    require_json = False

    def render_timeout_request_response(self, error_dict=None):
        if error_dict is None:
            error_dict = self.error_response_dict
        json_context = json.dumps(
            error_dict,
            cls=self.json_encoder_class,
            **self.get_json_dumps_kwargs()
        ).encode('utf-8')
        return HttpResponse(json_context, content_type=self.get_content_type(), status=502)

    def get_ajax(self, request, *args, **kwargs):
        session_id = request.GET.get('uuid')
        if not session_id:
            return self.render_json_response([])
        session = {
            "uuid": session_id,
        }

        try:
            session_obj = Session.objects.get(username=self.request.user, uuid=session_id)
            session['topic_title'] = session_obj.topic.title
            session['topic_number'] = session_obj.topic.number
            session['topic_description'] = session_obj.topic.description
            session['topic_seed_query'] = session_obj.topic.seed_query
            session['topic_narrative'] = session_obj.topic.narrative

            session['strategy'] = session_obj.get_strategy_display()
            session['effort'] = session_obj.max_number_of_judgments
            session['show_full_document_content'] = session_obj.show_full_document_content
            session['created_at'] = session_obj.created_at

            counters = Judgment.objects.filter(user=self.request.user,
                                               task=session_obj).aggregate(
                total_highlyRelevant=Count(Case(When(relevance=2, then=1))),
                total_relevant=Count(Case(When(relevance=1, then=1))),
                total_nonrelevant=Count(Case(When(relevance=0, then=1)))
            )

            session["total_highlyRelevant"] = counters["total_highlyRelevant"]
            session["total_nonrelevant"] = counters["total_nonrelevant"]
            session["total_relevant"] = counters["total_relevant"]
            session["total_judged"] = counters["total_highlyRelevant"] + counters["total_nonrelevant"] + counters["total_relevant"]

        except TimeoutError:
            error_msg = {u"message": u"Timeout error. Please check status of servers."}
            return JsonResponse(error_msg, status=408)
        except Session.DoesNotExist:
            return JsonResponse({"message": "Session not found."}, status=404)

        return self.render_json_response(session)


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
        practice_group, created = Group.objects.get_or_create(name='practice')

        # Create practice user and save to the database
        lst = [random.choice(string.ascii_letters + string.digits) for n in range(5)]
        randomstr = "".join(lst)
        username = password = "{}_practice".format(randomstr)
        User = get_user_model()
        practice_user = User.objects.create_user(username,
                                                 '{}@crazymail.com'.format(username),
                                                 password)

        practice_group.user_set.add(practice_user)
        practice_group.save()

        credentials = {
            "username": username,
            "password": password
        }

        user = get_adapter(self.request).authenticate(
            self.request,
            **credentials)
        ret = perform_login(request, user,
                            email_verification=False,
                            redirect_url=reverse_lazy('progress:home'))
        return ret
