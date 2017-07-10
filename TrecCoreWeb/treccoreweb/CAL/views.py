import json

from braces import views
from django.http import HttpResponseRedirect, HttpResponse
from django.urls import reverse_lazy
from django.views import generic

from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.CAL.logging_messages import LOGGING_MESSAGES as CAL_LOGGING_MESSAGES
from interfaces.DocumentSnippetEngine import functions as DocEngine
from treccoreweb.CAL.exceptions import CALError

import logging
logger = logging.getLogger(__name__)


class CALHomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'CAL/CAL.html'

    def get(self, request, *args, **kwargs):
        # TODO: If we're not going to use electron.js, make sure the view
        # is only allowed to people with permission to access this page
        current_task = self.request.user.current_task
        if current_task.is_time_past():
            return HttpResponseRedirect(reverse_lazy('progress:completed'))

        return super(CALHomePageView, self).get(self, request, *args, **kwargs)


class CALCtrlFAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            search_field_value = self.request_json.get(u"search_field_value")
            page_title = self.request_json.get(u"page_title")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, "
                                      u"and search_field_value"}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": CAL_LOGGING_MESSAGES.get("ctrlf", None),
                "searchfield_input": search_field_value,
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your event has been recorded"}
        return self.render_json_response(context)


class CALMessageAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
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
            doc_CAL_snippet = self.request_json.get(u'doc_CAL_snippet')
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
                "doc_CAL_snippet": doc_CAL_snippet,
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
    View to get a list of documents to judge from CAL
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
        session = self.request.user.current_task.uuid
        seed_query = self.request.user.current_task.topic.seed_query
        try:
            docs_ids_to_judge, top_terms = CALFunctions.get_documents(str(session), 5,
                                                                      seed_query)
            if not docs_ids_to_judge:
                return self.render_json_response([])

            doc_ids_hack = []
            for doc_id in docs_ids_to_judge:
                doc = {'doc_id': doc_id}
                if '.' in doc_id:
                    doc['doc_id'], doc['para_id'] = doc_id.split('.')
                doc_ids_hack.append(doc)

            documents = DocEngine.get_documents_with_snippet(doc_ids_hack,
                                                             seed_query,
                                                             top_terms)
            return self.render_json_response(documents)
        except TimeoutError:
            error_dict = {u"message": u"Timeout error. Please check status of servers."}
            return self.render_timeout_request_response(error_dict)
        except CALError as e:
            log_body = {
                "user": self.request.user.username,
                "result": {
                    "message": str(e),
                    "source": "interfaces.CAL.functions.get_documents()"
                }
            }

            logger.error("[{}]".format(log_body))
            error_dict = {u"message": u"Error occurred. Please inform study coordinators"}

            # TODO: add proper http response for CAL errors
            return self.render_timeout_request_response(error_dict)
