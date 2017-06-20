import json

from braces import views
from django.db.models import Count, Case, When
from django.http import HttpResponseBadRequest, HttpResponse

from django.views import generic
from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.judgment.models import Judgement
from treccoreweb.CAL.logging_messages import LOGGING_MESSAGES as CAL_LOGGING_MESSAGES
from interfaces.DocumentSnippetEngine import functions as DocEngine

import logging
logger = logging.getLogger(__name__)


class CALHomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'CAL/CAL.html'

    def get_context_data(self, **kwargs):
        context = super(CALHomePageView, self).get_context_data(**kwargs)
        counters = Judgement.objects.filter(user=self.request.user,
                                    topic=self.request.user.current_task.topic).aggregate(
            total_relevant=Count(Case(When(relevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_ontopic=Count(Case(When(ontopic=True, then=1)))
        )
        context["total_relevant"] = counters["total_relevant"]
        context["total_nonrelevant"] = counters["total_nonrelevant"]
        context["total_ontopic"] = counters["total_ontopic"]

        return context


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


class CALVisitAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, page_title"}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": CAL_LOGGING_MESSAGES.get("visit", None),
                "page_visit": True,
                "page_file": "CAL.html",
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your visit has been recorded"}
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
        session = self.request.user.current_task.topic.uuid
        seed_query = self.request.user.current_task.topic.seed_query
        try:
            docs_ids_to_judge, top_terms = CALFunctions.get_documents(str(session), 5,
                                                                      seed_query)
            if not docs_ids_to_judge:
                return self.render_json_response([])
            documents = DocEngine.get_documents_with_snippet(docs_ids_to_judge,
                                                             seed_query,
                                                             top_terms)
        except TimeoutError:
            error_dict = {u"message": u"Timeout error. Please check status of servers."}
            return self.render_timeout_request_response(error_dict)

        return self.render_json_response(documents)
