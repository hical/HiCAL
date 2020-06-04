import logging

from braces.views import LoginRequiredMixin
from django.contrib import messages
from django.http import HttpResponseRedirect
from django.shortcuts import render
from django.urls import reverse_lazy
from django.views import generic

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.topic.forms import TopicForm
from hicalweb.topic.models import Topic

logger = logging.getLogger(__name__)


class TopicView(LoginRequiredMixin, generic.DetailView):
    model = Topic
    template_name = "topic/details.html"
    object = None

    def get_context_data(self, **kwargs):
        context = {
            "form": TopicForm(instance=self.request.user.current_session.topic)
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

        try:
            CALFunctions.add_session(str(self.object.uuid),
                                     self.object.topic.seed_query,
                                     self.object.strategy)
            self.object.save()
            messages.add_message(self.request,
                                 messages.SUCCESS,
                                 'Your topic has been created but it\'s not active. '
                                 'Activate it to start working under it')
        except CALError as e:
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
        request.user.current_session.topic = topic
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
