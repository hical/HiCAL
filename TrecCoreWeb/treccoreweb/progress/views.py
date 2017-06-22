from braces import views
from treccoreweb.topic.models import *
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic
from treccoreweb.progress.models import Demographic, PreTask, PostTask
from treccoreweb.progress.forms import DemographicForm, PreTaskForm, PostTaskForm
from django.shortcuts import render, get_object_or_404

import logging
logger = logging.getLogger(__name__)


class Home(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/home.html'

    def get_context_data(self, **kwargs):
        context = super(Home, self).get_context_data(**kwargs)
        # TODO: Get any related context here
        return context

    def get(self, request, *args, **kwargs):
        current_task = self.request.user.current_task
        # if first task and pre-task is not done, show tutorial.
        if current_task.is_first_task() and not current_task.pretask.is_completed:
            return HttpResponseRedirect(reverse_lazy('progress:tutorial'))
        # if the current task is the last one and it has been completed, go to exit
        elif current_task.is_last_task() and current_task.is_completed():
            return HttpResponseRedirect(reverse_lazy('progress:exit'))
        # if user time in task is over, move to post-task
        elif current_task.is_time_past():
            return HttpResponseRedirect(reverse_lazy('progress:posttask'))

        return super(Home, self).get(self, request, *args, **kwargs)


class DemographicCreateView(views.LoginRequiredMixin, generic.CreateView):
    model = Demographic
    template_name = "progress/demographic.html"
    object = None
    form_class = DemographicForm
    success_url = reverse_lazy("progress:pretask")

    def form_valid(self, form):
        prev = Demographic.objects.filter(username=self.request.user)
        # if a demographic instance already exists, delete old one
        if prev:
            prev.first().delete()
        self.object = form.save(commit=False)
        self.object.username = self.request.user
        return super(DemographicCreateView, self).form_valid(form)


class PretaskView(views.LoginRequiredMixin, generic.CreateView):
    model = PreTask
    template_name = "progress/pretask.html"
    object = None

    def get_context_data(self, **kwargs):
        context = {
            "form": PreTaskForm(instance=self.request.user.current_task.pretask)
        }

        return context

    def get(self, request, *args, **kwargs):
        # if user already completed the pre-task, move to main task
        if self.request.user.current_task.pretask.is_completed:
            return HttpResponseRedirect(reverse_lazy('progress:home'))

        return super(PretaskView, self).get(self, request, *args, **kwargs)

    def get_object(self):
        pk = self.request.user.current_task.pretask.id
        return get_object_or_404(PreTask, pk=pk)

    def get_success_url(self):
        return reverse_lazy('progress:home')

    def post(self, request, *args, **kwargs):
        unmodified_object = self.get_object()
        self.object = self.get_object()
        context = self.get_context_data()

        if 'submit-pretask-form' in request.POST:
            form = PreTaskForm(request.POST,
                               instance=self.object)
            context['form'] = form
            if form.is_valid():
                j = form.save(commit=False)
                # Mark pre-task as completed
                self.object.is_completed = True
                j.save()
            else:
                context['object'] = unmodified_object
                return render(request,
                              template_name=self.template_name,
                              context=context
                              )

        return HttpResponseRedirect(self.get_success_url())


class PosttaskView(views.LoginRequiredMixin, generic.CreateView):
    model = PostTask
    template_name = "progress/posttask.html"
    object = None

    def get_context_data(self, **kwargs):
        context = {
            "form": PostTaskForm(instance=self.request.user.current_task.posttask)
        }

        return context

    def get(self, request, *args, **kwargs):
        # if user haven't completed the main task, move to main task
        if not self.request.user.current_task.is_time_past():
            return HttpResponseRedirect(reverse_lazy('progress:home'))

        return super(PosttaskView, self).get(self, request, *args, **kwargs)

    def get_object(self):
        pk = self.request.user.current_task.posttask.id
        return get_object_or_404(PostTask, pk=pk)

    def get_success_url(self):
        return reverse_lazy('progress:home')

    def post(self, request, *args, **kwargs):
        unmodified_object = self.get_object()
        self.object = self.get_object()
        context = self.get_context_data()

        if 'submit-posttask-form' in request.POST:
            form = PostTaskForm(request.POST,
                                instance=self.object)
            context['form'] = form
            if form.is_valid():
                j = form.save(commit=False)
                # Mark post-task as completed
                self.object.is_completed = True
                j.save()

                # Move user current task to the next one
                next_task = self.request.user.current_task.next_task()

                if next_task is not None:
                    self.request.user.current_task = next_task
                    self.request.user.save()
                # if user has completed all tasks, move to exit questionnaire
                else:
                    return HttpResponseRedirect(reverse_lazy('progress:exit'))
            else:
                context['object'] = unmodified_object
                return render(request,
                              template_name=self.template_name,
                              context=context
                              )

        return HttpResponseRedirect(self.get_success_url())
