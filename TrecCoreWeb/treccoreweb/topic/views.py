from braces.views import LoginRequiredMixin
from django.contrib import messages
from django.http import HttpResponseRedirect
from django.shortcuts import render
from django.urls import reverse_lazy
from django.views import generic
from treccoreweb.topic.models import Topic
from treccoreweb.topic.forms import TopicForm
from treccoreweb.topic.logging_messages import LOGGING_MESSAGES as TOPIC_LOGGING_MESSAGES
from treccoreweb.interfaces.CAL import functions as CALFunctions

import logging
logger = logging.getLogger(__name__)

from braces import views

class TopicView(LoginRequiredMixin, generic.DetailView):
    model = Topic
    template_name = "topic/details.html"
    object = None

    def get_context_data(self, **kwargs):
        context = {
            "form": TopicForm(instance=self.request.user.current_topic)
        }

        return context

    def get_success_url(self):
        return reverse_lazy('topic:detail', kwargs={'pk': self.object.id})

    def post(self, request, *args, **kwargs):
        unmodified_object = self.get_object()
        self.object = self.get_object()
        context = self.get_context_data()

        if 'submit-topic-form' in request.POST:
            form = TopicForm(request.POST,
                             instance=self.object)
            context['form'] = form
            if form.is_valid():
                j = form.save(commit=False)
                j.save()
                messages.add_message(request,
                                     messages.SUCCESS,
                                     'Your topic has been updated.')
            else:
                context['object'] = unmodified_object
                return render(request,
                              template_name=self.template_name,
                              context=context
                              )

        return HttpResponseRedirect(self.get_success_url())


class TopicVisitAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                         views.JsonRequestResponseMixin,
                         generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time", None)
            type = self.request_json.get("type", None)
        except KeyError:
            error_dict = {u"message": u"your input must include client_time."}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": TOPIC_LOGGING_MESSAGES.get("visit").get(type, None),
                "page_visit": True,
                "page_file": "{}.html".format(type),
                "page_title": "Topics {}".format(type)
            }
        }

        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your visit has been recorded."}
        return self.render_json_response(context)


class TopicCreateView(LoginRequiredMixin, generic.CreateView):
    model = Topic
    template_name = "topic/create.html"
    form_class = TopicForm
    object = None

    def get_success_url(self):
        return reverse_lazy('topic:list')

    def form_valid(self, form):
        self.object = form.save(commit=False)
        self.object.username = self.request.user

        if CALFunctions.add_session(str(self.object.uuid), self.object.seed_query):
            self.object.save()
            messages.add_message(self.request,
                                 messages.SUCCESS,
                                 'Your topic has been created but it\'s not active. '
                                 'Activate it to start working under it')
        else:
            messages.add_message(self.request,
                                 messages.ERROR,
                                 'Failed to create session. CAL backend failed to add  '
                                 'session')

        return HttpResponseRedirect(self.get_success_url())


class TopicListView(LoginRequiredMixin, generic.ListView):
    model = Topic
    template_name = "topic/list.html"

    def get_queryset(self):
        return super(TopicListView, self).get_queryset().filter(username=self.request.user)

    def form_valid(self, form):
        self.object = form.save(commit=False)
        self.object.username = self.request.user
        self.object.save()
        return HttpResponseRedirect(self.get_success_url())


class TopicActivateView(LoginRequiredMixin, generic.DetailView):
    model = Topic

    def get_success_url(self):
        return reverse_lazy('topic:list')

    def post(self, request, *args, **kwargs):
        topic_id = request.POST.get("topic_id")
        topic = Topic.objects.get(id=topic_id, username=request.user)
        request.user.current_topic = topic
        request.user.save()
        messages.add_message(request,
                             messages.SUCCESS,
                             'Your active topic has been updated.')
        return HttpResponseRedirect(self.get_success_url())


class TopicDeleteView(LoginRequiredMixin, generic.DetailView):
    model = Topic

    def get_success_url(self):
        return reverse_lazy('topic:list')

    def post(self, request, *args, **kwargs):
        topic_id = request.POST.get("topic_id")
        topic = Topic.objects.get(id=topic_id, username=request.user)
        topic.delete()
        messages.add_message(request,
                             messages.SUCCESS,
                             'Your topic has been deleted.')
        return HttpResponseRedirect(self.get_success_url())


