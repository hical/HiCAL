from braces import views
from django.views import generic
from treccoreweb.interfaces.CAL import functions as CALFunctions
from treccoreweb.judgment.models import Judgement
import logging
logger = logging.getLogger(__name__)


class JudgmentAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                       views.JsonRequestResponseMixin,
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
                        "session": self.request.user.current_topic.uuid,
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
                        "session": self.request.user.current_topic.uuid,
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
            next_patch = CALFunctions.send_judgment(self.request.user.current_topic.uuid,
                                                    doc_id)
            context[u"next_docs"] = next_patch
            return self.render_json_response(context)
        else:
            return self.render_json_response(context)
