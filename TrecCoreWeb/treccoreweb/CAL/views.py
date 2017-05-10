from braces import views
from django.db.models import Count, Case, When

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
                                    topic=self.request.user.current_topic).aggregate(
            total_relevant=Count(Case(When(relevant=True, then=1))),
            total_nonrelevant=Count(Case(When(nonrelevant=True, then=1))),
            total_notsure=Count(Case(When(notsure=True, then=1)))
        )
        context["total_relevant"] = counters["total_relevant"]
        context["total_nonrelevant"] = counters["total_nonrelevant"]
        context["total_notsure"] = counters["total_notsure"]

        return context


class CALCtrlFAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            search_field_value = self.request_json.get(u"search_field_value")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, "
                                      u"and search_field_value"}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": CAL_LOGGING_MESSAGES.get("ctrlf", None),
                "searchfield_input": search_field_value
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
        except KeyError:
            error_dict = {u"message": u"your input must include client_time"}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": CAL_LOGGING_MESSAGES.get("visit", None),
                "page_visit": True,
                "page_file": "CAL.html",
                "page_title": "CAL"
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

    def get_ajax(self, request, *args, **kwargs):
        session = self.request.user.current_topic.uuid
        seed_query = self.request.user.current_topic.seed_query
        docs_ids_to_judge = CALFunctions.get_documents(str(session), 5, seed_query)
        documents = DocEngine.get_documents(docs_ids_to_judge, seed_query)
        return self.render_json_response(documents)
