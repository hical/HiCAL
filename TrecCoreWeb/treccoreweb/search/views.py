import httplib2
from braces import views
from django.db.models import Count, Case, When
from django.http import HttpResponse
from django.template import loader

from django.views import generic
from treccoreweb.judgment.models import Judgement
from treccoreweb.search import helpers
import logging

from treccoreweb.search.logging_messages import LOGGING_MESSAGES as SEARCH_LOGGING_MESSAGES
from treccoreweb.interfaces.SearchEngine.functions import get_documents

logger = logging.getLogger(__name__)


class SearchHomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'search/search.html'

    def get_context_data(self, **kwargs):
        context = super(SearchHomePageView, self).get_context_data(**kwargs)
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


class SearchVisitAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, page title."}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": SEARCH_LOGGING_MESSAGES.get("visit", None),
                "page_visit": True,
                "page_file": "search.html",
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your visit has been recorded."}
        return self.render_json_response(context)


class SearchInputStatusAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            isFocused = self.request_json.get(u"isFocused")
            page_title = self.request_json.get(u"page_title")
            search_bar_value = self.request_json.get(u"search_bar_value")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time, page_title"
                                      u"search bar value, and isFocused."}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": SEARCH_LOGGING_MESSAGES.get("search_input", None),
                "isFocused": isFocused,
                "search_bar_value": search_bar_value,
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your search input event has been recorded."}
        return self.render_json_response(context)


class SearchKeystrokeAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
            character = self.request_json.get(u"character")
            search_bar_value = self.request_json.get(u"search_bar_value")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u" page title, character, and search bar value."}
            return self.render_bad_request_response(error_dict)

        log_body = {
            "user": self.request.user.username,
            "client_time": client_time,
            "result": {
                "message": SEARCH_LOGGING_MESSAGES.get("keystroke", None),
                "character": character,
                "search_bar_value": search_bar_value,
                "page_title": page_title
            }
        }
        logger.info("[{}]".format(log_body))

        context = {u"message": u"Your visit has been recorded."}
        return self.render_json_response(context)


class SearchListView(views.CsrfExemptMixin, generic.base.View):
    template = 'search/search_list.html'

    def post(self, request, *args, **kwargs):
        template = loader.get_template(self.template)
        try:
            search_input = request.POST.get("search_input")
        except KeyError:
            rendered_template = template.render({})
            return HttpResponse(rendered_template, content_type='text/html')
        context = {}
        documents_values, document_ids = None, None
        try:
            documents_values, document_ids = get_documents(search_input)
        except (TimeoutError, httplib2.HttpLib2Error):
            context['error'] = "Error happened. Please check search server."

        if document_ids:
            document_ids = helpers.padder(document_ids)

            documents_values = helpers.join_judgments(documents_values, document_ids,
                                                      self.request.user,
                                                      self.request.user.current_topic)

        context["documents"] = documents_values

        rendered_template = template.render(context)
        return HttpResponse(rendered_template, content_type='text/html')

