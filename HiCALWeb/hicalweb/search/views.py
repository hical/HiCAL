import json
import logging

from braces import views
from django.http import HttpResponse
from django.template import loader
from django.views import generic
import httplib2

from hicalweb.interfaces.DocumentSnippetEngine import functions as DocEngine
from hicalweb.interfaces.SearchEngine import functions as SearchEngine
from hicalweb.search import helpers

logger = logging.getLogger(__name__)


class SearchHomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'search/search.html'

    def get(self, request, *args, **kwargs):
        return super(SearchHomePageView, self).get(self, request, *args, **kwargs)


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
            isSearchbarFocused = self.request_json.get(u"isSearchbarFocused")
            search_bar_value = self.request_json.get(u"search_bar_value")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u" page title, character, isSearchbarFocused,"
                                      u" and search bar value."}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your keystroke has been recorded."}
        return self.render_json_response(context)


class SearchListView(views.CsrfExemptMixin, generic.base.View):
    template = 'search/search_list.html'

    def post(self, request, *args, **kwargs):
        template = loader.get_template(self.template)
        try:
            search_input = request.POST.get("search_input")
            numdisplay = request.POST.get("numdisplay", 10)
        except KeyError:
            rendered_template = template.render({})
            return HttpResponse(rendered_template, content_type='text/html')
        context = {}
        documents_values, document_ids = None, None
        try:
            documents_values, document_ids, total_time = SearchEngine.get_documents(
                                                            search_input,
                                                            numdisplay=numdisplay
                                                         )
        except (TimeoutError, httplib2.HttpLib2Error):
            context['error'] = "Error happened. Please check search server."

        if document_ids:
            # document_ids = helpers.padder(document_ids)
            documents_values = helpers.join_judgments(documents_values, document_ids,
                                                      self.request.user,
                                                      self.request.user.current_task)

        context["documents"] = documents_values
        context["query"] = search_input
        if total_time:
            context["total_time"] = "{0:.2f}".format(round(float(total_time), 2))

        rendered_template = template.render(context)
        return HttpResponse(rendered_template, content_type='text/html')


class SearchButtonView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
            query = self.request_json.get(u"query")
            numdisplay = self.request_json.get(u"numdisplay")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u" page title, query, and numdisplay values"}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your search request has been recorded."}
        return self.render_json_response(context)


class SearchSearchDocOpenedView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            client_time = self.request_json.get(u"client_time")
            page_title = self.request_json.get(u"page_title")
            query = self.request_json.get(u"query")
            doc_id = self.request_json.get(u"doc_id")
            doc_title = self.request_json.get(u"doc_title")
            doc_search_snippet = self.request_json.get(u"doc_search_snippet")
        except KeyError:
            error_dict = {u"message": u"your input must include client_time,"
                                      u" page title, query, and docid values"}
            return self.render_bad_request_response(error_dict)

        context = {u"message": u"Your document click request has been recorded."}
        return self.render_json_response(context)


class SearchGetDocAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
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
        return HttpResponse(
            json_context, content_type=self.get_content_type(), status=502)

    def get_ajax(self, request, *args, **kwargs):
        docid = request.GET.get('docid')
        if not docid:
            return self.render_json_response([])
        try:
            document = DocEngine.get_documents([docid])
        except TimeoutError:
            error_dict = {u"message": u"Timeout error. Please check status of servers."}
            return self.render_timeout_request_response(error_dict)

        return self.render_json_response(document)
