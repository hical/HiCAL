import logging

from braces import views
from django.http import HttpResponseRedirect
from django.shortcuts import get_object_or_404
from django.shortcuts import render
from django.urls import reverse_lazy
from django.views import generic

from treccoreweb.progress.forms import DemographicForm
from treccoreweb.progress.forms import PostTaskForm
from treccoreweb.progress.forms import PreTaskForm
from treccoreweb.progress.logging_messages import \
    LOGGING_MESSAGES as PROGRESS_LOGGING_MESSAGES
from treccoreweb.progress.models import Demographic
from treccoreweb.progress.models import PostTask
from treccoreweb.progress.models import PreTask

logger = logging.getLogger(__name__)


class Home(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/home.html'

    def get_context_data(self, **kwargs):
        context = super(Home, self).get_context_data(**kwargs)
        # TODO: Get any related context here
        return context

    def get(self, request, *args, **kwargs):
        current_task = self.request.user.current_task
        # if demographic is not completed yet, show demographic
        if not Demographic.objects.filter(username=self.request.user):
            return HttpResponseRedirect(reverse_lazy('progress:demographic'))
        # if first task and pre-task is not done, show tutorial.
        elif current_task.is_first_task() and not current_task.pretask.is_completed:
            return HttpResponseRedirect(reverse_lazy('progress:tutorial'))
        # if the current task is the last one and it has been completed, go to exit
        elif current_task.is_last_task() and current_task.is_completed():
            return HttpResponseRedirect(reverse_lazy('progress:exit'))
        # if user time in task is over, move to post-task
        elif current_task.is_time_past() and not current_task.is_iterative():
            return HttpResponseRedirect(reverse_lazy('progress:posttask'))
        # if user is in iterative mode and have judged all documents
        elif current_task.is_iterative() and current_task.is_iterative_completed():
            return HttpResponseRedirect(reverse_lazy('progress:posttask'))

        return super(Home, self).get(self, request, *args, **kwargs)


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

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": "{} page visit".format(page_file),
                "page_visit": True,
                "page_file": page_file,
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

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

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": "Ctrl+f event",
                "extra_context": extra_context,
                "searchfield_input": search_field_value,
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

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

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": PROGRESS_LOGGING_MESSAGES.get("find_keystroke", None),
                "character": character,
                "search_bar_value": search_bar_value,
                "isSearchbarFocused": isSearchbarFocused,
                'page_title': page_title,
                "doc_id": doc_id
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your visit has been recorded."}
        return self.render_json_response(context)


class DemographicCreateView(views.LoginRequiredMixin, generic.CreateView):
    model = Demographic
    template_name = "progress/demographic.html"
    object = None
    form_class = DemographicForm
    success_url = reverse_lazy("progress:tutorial")

    def form_valid(self, form):
        prev = Demographic.objects.filter(username=self.request.user)
        # if a demographic instance already exists, delete old one
        if prev:
            prev.first().delete()
        self.object = form.save(commit=False)
        self.object.username = self.request.user
        return super(DemographicCreateView, self).form_valid(form)


class TutorialView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = "progress/tutorial.html"


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


class Completed(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'progress/task_completed.html'

    def get_context_data(self, **kwargs):
        context = super(Completed, self).get_context_data(**kwargs)
        # TODO: Get any related context here
        return context

    def get(self, request, *args, **kwargs):
        current_task = self.request.user.current_task
        # check if in iterative mode and completed all documents
        iterative_mode_check = False
        if current_task.is_iterative and current_task.is_iterative_completed:
            iterative_mode_check = True
        # check if current task is not completed, if yes, go to home page
        if not current_task.is_time_past() and not iterative_mode_check:
            return HttpResponseRedirect(reverse_lazy('progress:home'))

        return super(Completed, self).get(self, request, *args, **kwargs)
