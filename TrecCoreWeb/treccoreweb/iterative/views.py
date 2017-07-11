import json
import logging

from braces import views
from django.http import HttpResponse
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic
from interfaces.DocumentSnippetEngine import functions as DocEngine
from interfaces.Iterative import functions as IterativeEngine

logger = logging.getLogger(__name__)


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'iterative/iterative.html'

    def get(self, request, *args, **kwargs):
        # TODO: If we're not going to use electron.js, make sure the view
        # is only allowed to people with permission to access this page
        current_task = self.request.user.current_task
        if current_task.is_time_past():
            return HttpResponseRedirect(reverse_lazy('progress:completed'))

        return super(HomePageView, self).get(self, request, *args, **kwargs)


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
            doc_id = self.request_json.get(u'doc_id')
            extra_context = self.request_json.get(u'extra_context')
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, "
                                      u"message, ... etc"}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "action": action,
                "message": message,
                "doc_id": doc_id,
                "page_title": page_title,
                "extra_context": extra_context,
            }
        }

        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your log message with action '{}' and of "
                               u"document '{}' has been logged.".format(action, doc_id)}
        return self.render_json_response(context)


class DocAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                  views.JsonRequestResponseMixin,
                  views.AjaxResponseMixin, generic.View):
    """
    View to get a list of documents to judge
    """
    require_json = False

    def render_timeout_request_response(self, error_dict=None):
        if error_dict is None:
            error_dict = self.error_response_dict
        json_context = json.dumps(
            error_dict,
            cls=self.json_encoder_class,
            **self.get_json_dumps_kwargs()
        ).encode('utf-8')
        return HttpResponse(
            json_context, content_type=self.get_content_type(), status=502)

    def get_ajax(self, request, *args, **kwargs):
        try:
            docs_ids = IterativeEngine.get_documents(self.request.user.id)
            documents = DocEngine.get_documents(docs_ids, query=None)
            return self.render_json_response(documents)
        except TimeoutError:
            error_dict = {u"message": u"Timeout error. Please check status of servers."}
            return self.render_timeout_request_response(error_dict)
