from braces import views
from django.db.models import Count, Case, When
from django.views import generic
from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.judgment.models import Judgement

import logging
logger = logging.getLogger(__name__)


class CALHomePageView(generic.TemplateView):
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


class DocAJAXView(views.CsrfExemptMixin, views.JsonRequestResponseMixin,
                  views.AjaxResponseMixin, generic.View):
    """
    View to get a list of documents to judge from CAL
    """
    require_json = False

    def get_ajax(self, request, *args, **kwargs):
        session = self.request.user.current_topic.uuid
        seed_query = self.request.user.current_topic.seed_query
        docs_ids_to_judge = CALFunctions.get_documents(session, 5, seed_query)

        return self.render_json_response(docs_ids_to_judge)


class JudgmentAJAXView(views.CsrfExemptMixin, views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            doc_id = self.request_json[u"doc_id"]
            relevant = self.request_json[u"relevant"]
            nonrelevant = self.request_json[u"nonrelevant"]
            notsure = self.request_json[u"notsure"]
            time_to_judge = self.request_json[u"time_to_judge"]
            isFromCAL = self.request_json[u"isFromCAL"]
            query = self.request_json.get(u"query", None)
        except KeyError:
            error_dict = {u"message": u"your input must include doc_id, relevant,"
                                      u" nonrelevant, notsure, time_to_judge,"
                                      u" and session values"}
            return self.render_bad_request_response(error_dict)

        # TODO: Save judgment and update CAL
        Judgement.objects.create(
            user=self.request.user,
            doc_id=doc_id,
            topic=self.request.user.current_topic,
            query=query,
            relevant=relevant,
            nonrelevant=nonrelevant,
            notsure=notsure,
            time_to_judge=time_to_judge,
            isFromCAL=isFromCAL
        )

        logger.info("Saved doc '%s' judgment for user '%s'.",
                    doc_id, self.request.user)

        context = {u"message": u"Your judgment on {} has been received!".format(doc_id)}
        if isFromCAL:
            # TODO: return next 5 documents to judge
            next_patch = CALFunctions.send_judgment(self.request.user.current_topic.uuid,
                                                    5)
            context[u"next_docs"] = next_patch
            return self.render_json_response(context)
        else:
            return self.render_json_object_response(context)
