import json

from braces import views
from django.http import HttpResponse
from django.template import loader
from django.views import generic
from treccoreweb.interfaces.CAL import functions as CALFunctions
from interfaces.DocumentSnippetEngine import functions as DocEngine
from treccoreweb.judgment.models import Judgement

import logging

logger = logging.getLogger(__name__)


class JudgmentAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
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

    def post(self, request, *args, **kwargs):
        try:
            doc_id = self.request_json[u"doc_id"]
            doc_title = self.request_json[u"doc_title"]
            doc_CAL_snippet = self.request_json[u"doc_CAL_snippet"]
            doc_search_snippet = self.request_json[u"doc_search_snippet"]
            relevant = self.request_json[u"relevant"]
            nonrelevant = self.request_json[u"nonrelevant"]
            ontopic = self.request_json[u"ontopic"]
            time_to_judge = self.request_json[u"time_to_judge"]
            isFromCAL = self.request_json[u"isFromCAL"]
            isFromSearch = self.request_json[u"isFromSearch"]
            isFromSearchModal = self.request_json[u"isFromSearchModal"]
            fromMouse = self.request_json[u"fromMouse"]
            fromKeyboard = self.request_json[u"fromKeyboard"]
            query = self.request_json.get(u"query", None)
            client_time = self.request_json.get(u"client_time", None)
        except KeyError:
            error_dict = {u"message": u"your input must include doc_id, doc_title, "
                                      u"relevant, nonrelevant, ontopic, time_to_judge, "
                                      u"doc_CAL_snippet, doc_search_snippet, etc.."}
            return self.render_bad_request_response(error_dict)

        # Check if a judgment exists already, if so, update the db row.
        exists = Judgement.objects.filter(user=self.request.user,
                                          doc_id=doc_id,
                                          topic=self.request.user.current_topic)

        if exists:
            exists = exists.first()
            exists.query = query
            exists.doc_title = doc_title
            if doc_CAL_snippet != "":
                exists.doc_CAL_snippet = doc_CAL_snippet
            if doc_search_snippet != "":
                exists.doc_search_snippet = doc_search_snippet
            exists.relevant = relevant
            exists.nonrelevant = nonrelevant
            exists.ontopic = ontopic
            exists.time_to_judge = time_to_judge
            exists.isFromCAL = isFromCAL
            exists.isFromSearch = isFromSearch
            exists.isFromSearchModal = isFromSearchModal
            exists.fromMouse = fromMouse
            exists.fromKeyboard = fromKeyboard
            exists.save()

            log_body = {
                "user": self.request.user.username,
                "client_time": client_time,
                "result": {
                    "message": Judgement.LOGGING_MESSAGES.get("update", None),
                    "action": "update",
                    "doc_judgment": {
                        "doc_id": doc_id,
                        "doc_title": doc_title,
                        "topic_id": self.request.user.current_topic.id,
                        "session": str(self.request.user.current_topic.uuid),
                        "query": query,
                        "relevant": relevant,
                        "nonrelevant": nonrelevant,
                        "ontopic": ontopic,
                        "time_to_judge": time_to_judge,
                        "isFromCAL": isFromCAL,
                        "isFromSearch": isFromSearch,
                        "isFromSearchModal": isFromSearchModal,
                        "fromMouse": fromMouse,
                        "fromKeyboard": fromKeyboard
                    }
                }
            }

            logger.info("[{}]".format(log_body))
        else:
            Judgement.objects.create(
                user=self.request.user,
                doc_id=doc_id,
                doc_title=doc_title,
                doc_CAL_snippet=doc_CAL_snippet,
                doc_search_snippet=doc_search_snippet,
                topic=self.request.user.current_topic,
                query=query,
                relevant=relevant,
                nonrelevant=nonrelevant,
                ontopic=ontopic,
                time_to_judge=time_to_judge,
                isFromCAL=isFromCAL,
                isFromSearch=isFromSearch,
                isFromSearchModal=isFromSearchModal,
                fromMouse=fromMouse,
                fromKeyboard=fromKeyboard
            )

            log_body = {
                "user": self.request.user.username,
                "client_time": client_time,
                "result": {
                    "message": Judgement.LOGGING_MESSAGES.get("create", None),
                    "action": "create",
                    "doc_judgment": {
                        "doc_id": doc_id,
                        "doc_title": doc_title,
                        "topic_id": self.request.user.current_topic.id,
                        "session": str(self.request.user.current_topic.uuid),
                        "query": query,
                        "relevant": relevant,
                        "nonrelevant": nonrelevant,
                        "ontopic": ontopic,
                        "time_to_judge": time_to_judge,
                        "isFromCAL": isFromCAL,
                        "isFromSearch": isFromSearch,
                        "isFromSearchModal": isFromSearchModal,
                        "fromMouse": fromMouse,
                        "fromKeyboard": fromKeyboard
                    }
                }
            }

            logger.info("[{}]".format(log_body))

        context = {u"message": u"Your judgment on {} has been received!".format(doc_id)}
        if isFromCAL:
            # mark on topic documents as relevant only to CAL.
            rel = 1 if relevant else -1 if nonrelevant else 1
            try:
                next_patch, top_terms = CALFunctions.send_judgment(
                    self.request.user.current_topic.uuid,
                    doc_id,
                    rel)
                if not next_patch:
                    return self.render_json_response([])

                doc_ids_hack = []
                for doc_id in next_patch:
                    doc = {'doc_id': doc_id}
                    if '.' in doc_id:
                        doc['doc_id'], doc['para_id'] = doc_id.split('.')
                    doc_ids_hack.append(doc)

                documents = DocEngine.get_documents_with_snippet(doc_ids_hack,
                                                    self.request.user.current_topic.seed_query,
                                                    top_terms)
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)

            context[u"next_docs"] = documents
            return self.render_json_response(context)
        else:
            # mark on topic documents as relevant only to CAL.
            rel = 1 if relevant else -1 if nonrelevant else 1
            try:
                CALFunctions.send_judgment(self.request.user.current_topic.uuid,
                                           doc_id,
                                           rel)
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)

            return self.render_json_response(context)


class GetLatestAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
                       generic.View):
    require_json = False

    def get(self, request, number_of_docs_to_show, *args, **kwargs):
        try:
            number_of_docs_to_show = int(number_of_docs_to_show)
        except ValueError:
            return self.render_json_response([])

        latest = Judgement.objects.filter(
                    user=self.request.user,
                    topic=self.request.user.current_topic,
                    isFromCAL=True
                 ).order_by('-updated_at')[:number_of_docs_to_show]
        result = []
        for judgment in latest:
            result.append(
                {
                    "doc_id": judgment.doc_id,
                    "doc_title": judgment.doc_title,
                    "doc_date": "",
                    "doc_CAL_snippet": judgment.doc_CAL_snippet,
                    "doc_content": "",
                    "relevant": judgment.relevant,
                    "nonrelevant": judgment.nonrelevant,
                    "ontopic": judgment.ontopic,
                }
            )

        return self.render_json_response(result)


class GetAllView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'judgment/all.html'

    def get_context_data(self, **kwargs):
        context = {
            "judgments_list": Judgement.objects.filter(
                    user=self.request.user,
                    topic=self.request.user.current_topic,
                 ).order_by('-updated_at')
        }

        return context
