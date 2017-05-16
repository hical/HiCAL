import json

from braces import views
from django.http import HttpResponse
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
            relevant = self.request_json[u"relevant"]
            nonrelevant = self.request_json[u"nonrelevant"]
            notsure = self.request_json[u"notsure"]
            time_to_judge = self.request_json[u"time_to_judge"]
            isFromCAL = self.request_json[u"isFromCAL"]
            fromMouse = self.request_json[u"fromMouse"]
            fromKeyboard = self.request_json[u"fromKeyboard"]
            query = self.request_json.get(u"query", None)
            client_time = self.request_json.get(u"client_time", None)
        except KeyError:
            error_dict = {u"message": u"your input must include doc_id, relevant, "
                                      u"nonrelevant, notsure, time_to_judge, "
                                      u"etc.."}
            return self.render_bad_request_response(error_dict)

        # TODO: Save judgment and update CAL
        exists = Judgement.objects.filter(user=self.request.user,
                                          doc_id=doc_id,
                                          topic=self.request.user.current_topic)

        if exists:
            exists = exists.first()
            exists.query = query
            exists.relevant = relevant
            exists.nonrelevant = nonrelevant
            exists.notsure = notsure
            exists.time_to_judge = time_to_judge
            exists.isFromCAL = isFromCAL
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
                        "topic_id": self.request.user.current_topic.id,
                        "session": str(self.request.user.current_topic.uuid),
                        "query": query,
                        "relevant": relevant,
                        "nonrelevant": nonrelevant,
                        "notsure": notsure,
                        "time_to_judge": time_to_judge,
                        "isFromCAL": isFromCAL,
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
                topic=self.request.user.current_topic,
                query=query,
                relevant=relevant,
                nonrelevant=nonrelevant,
                notsure=notsure,
                time_to_judge=time_to_judge,
                isFromCAL=isFromCAL,
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
                        "topic_id": self.request.user.current_topic.id,
                        "session": str(self.request.user.current_topic.uuid),
                        "query": query,
                        "relevant": relevant,
                        "nonrelevant": nonrelevant,
                        "notsure": notsure,
                        "time_to_judge": time_to_judge,
                        "isFromCAL": isFromCAL,
                        "fromMouse": fromMouse,
                        "fromKeyboard": fromKeyboard
                    }
                }
            }

            logger.info("[{}]".format(log_body))

        context = {u"message": u"Your judgment on {} has been received!".format(doc_id)}
        if isFromCAL:
            # TODO: return next 5 documents to judge
            rel = 1 if relevant else -1 if nonrelevant else 0
            try:
                next_patch, top_terms = CALFunctions.send_judgment(
                    self.request.user.current_topic.uuid,
                    doc_id,
                    rel)
                if not next_patch:
                    return self.render_json_response([])

                documents = DocEngine.get_documents_with_snippet(next_patch,
                                                    self.request.user.current_topic.seed_query,
                                                    top_terms)
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)

            context[u"next_docs"] = documents
            return self.render_json_response(context)
        else:
            rel = 1 if relevant else -1 if nonrelevant else 0
            try:
                CALFunctions.send_judgment(self.request.user.current_topic.uuid,
                                           doc_id,
                                           rel)
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)

            return self.render_json_response(context)
