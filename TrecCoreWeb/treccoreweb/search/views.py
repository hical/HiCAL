from braces import views
from django.db.models import Count, Case, When
from django.http import HttpResponse
from django.template import loader

from django.views import generic
from treccoreweb.judgment.models import Judgement
from treccoreweb.search import helpers
import logging
import urllib
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


class SearchListView(views.CsrfExemptMixin, generic.base.View):
    template = 'search/search_list.html'

    def post(self, request, *args, **kwargs):
        template = loader.get_template(self.template)
        try:
            search_input = request.POST.get("search_input")
        except KeyError:
            rendered_template = template.render({})
            return HttpResponse(rendered_template, content_type='text/html')

        documents_values, document_ids = get_documents(search_input)
        if document_ids:
            documents_values = helpers.join_judgments(documents_values, document_ids,
                                                      self.request.user,
                                                      self.request.user.current_topic)

        context = {
            "documents": documents_values
        }

        rendered_template = template.render(context)
        return HttpResponse(rendered_template, content_type='text/html')

