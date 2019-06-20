import json
import logging
import traceback

from braces import views
from django.contrib import messages
from django.http import HttpResponse
from django.views import generic
from interfaces.DocumentSnippetEngine import functions as DocEngine

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.judgment.models import Judgment

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
            rel = self.request_json[u"rel"]
            source = self.request_json.get(u"source", None)
            time_verbose = self.request_json.get(u"time_verbose", None)
            meta = self.request_json.get(u"meta", None)
        except KeyError:
            error_dict = {u"message": u"your input must include `doc_id` and `rel`"}

            return self.render_bad_request_response(error_dict)

        # Check if a judgment exists already, if so, update the db row.
        exists = Judgment.objects.filter(user=self.request.user,
                                         doc_id=doc_id,
                                         task=self.request.user.current_task)

        if exists:
            exists = exists.first()
            exists.rel = rel
            exists.source = source
            exists.time_verbose.append(time_verbose)
            exists.meta.append(meta)
            exists.save()
        else:
            Judgment.objects.create(
                user=self.request.user,
                doc_id=doc_id,
                task=self.request.user.current_task,
                rel=rel,
                source=source,
                time_verbose=[time_verbose],
                meta=[meta]
            )

        context = {u"message": u"Your judgment on {} has been received!".format(doc_id),
                   u"is_max_judged_reached": False}
        error_message = None
        relevance = 1 if rel > 0 else 0

        if source == "CAL":
            context[u"next_docs"] = []
            try:
                next_patch = CALFunctions.send_judgment(
                    self.request.user.current_task.uuid,
                    doc_id,
                    relevance)
                if not next_patch:
                    return self.render_json_response(context)

                # TODO: Need to do a more generic solution
                doc_ids_hack = []
                for doc_id in next_patch:
                    doc = {'doc_id': doc_id}
                    if '.' in doc_id:
                        doc['doc_id'], doc['para_id'] = doc_id.split('.')
                    doc_ids_hack.append(doc)

                if self.request.user.current_task.strategy == 'doc':
                    documents = DocEngine.get_documents(next_patch,
                                                        self.request.user.current_task.topic.seed_query)
                else:
                    documents = DocEngine.get_documents_with_snippet(doc_ids_hack,
                                                                     self.request.user.current_task.topic.seed_query)
                context[u"next_docs"] = documents
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)
            except CALError as e:
                error_message = "CAL Exception: {}".format(str(e))
            except Exception as e:
                error_message = str(e)

        elif source == "search":
            try:
                CALFunctions.send_judgment(self.request.user.current_task.uuid,
                                           doc_id,
                                           relevance)
            except TimeoutError:
                traceback.print_exc()
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)
            except CALError as e:
                traceback.print_exc()
                error_message = "CAL Exception: {}".format(str(e))
            except Exception as e:
                traceback.print_exc()
                error_message = str(e)

        if error_message:
            log_body = {
                "user": self.request.user.username,
                "error_message": error_message,
                "topic_number": self.request.user.current_task.topic.number,
                "session": str(self.request.user.current_task.uuid),
                "action": "create judgment"
            }

            logger.error("{}".format(json.dumps(log_body)))

        if not exists:
            # Check if user has judged `max_judged` documents in total.
            judgments = Judgment.objects.filter(user=self.request.user,
                                                task=self.request.user.current_task)
            max_judged = self.request.user.current_task.max_number_of_judgments
            # Exit task only if number of judgments reached max (and maxjudged is enabled)
            if len(judgments) >= max_judged > 0:
                self.request.user.current_task = None
                self.request.user.save()

                message = 'You have judged >={} (max number of judgment you have ' \
                          'specified for this task) documents.'.format(max_judged)
                messages.add_message(request,
                                     messages.SUCCESS,
                                     message)
                context[u"is_max_judged_reached"] = True

        return self.render_json_response(context)


class NoJudgmentAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                         views.JsonRequestResponseMixin,
                         generic.View):
    require_json = False

    def post(self, request, *args, **kwargs):
        try:
            doc_id = self.request_json[u"doc_id"]
            source = self.request_json.get(u"source", None)
            time_verbose = self.request_json.get(u"time_verbose", None)
            meta = self.request_json.get(u"meta", None)
        except KeyError:
            error_dict = {u"message": u"your input must include `doc_id`."}
            return self.render_bad_request_response(error_dict)

        # Check if a judgment exists already, if so, update the time_verbose and meta.
        exists = Judgment.objects.filter(user=self.request.user,
                                         doc_id=doc_id,
                                         task=self.request.user.current_task)

        if exists:
            exists = exists.first()
            exists.time_verbose.append(time_verbose)
            exists.meta.append(meta)
            exists.save()
        else:
            Judgment.objects.create(
                user=self.request.user,
                doc_id=doc_id,
                task=self.request.user.current_task,
                rel=None,
                source=source,
                time_verbose=[time_verbose],
                meta=[meta]
            )

        context = {u"message": u"Received no judgment signal for {} ".format(doc_id)}
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
        latest = Judgment.objects.filter(
            user=self.request.user,
            task=self.request.user.current_task,
        ).filter(rel__isnull=False,
                 ).order_by('-updated_at')[:number_of_docs_to_show]
        result = []
        for judgment in latest:
            print(judgment, judgment.meta)
            if judgment.meta and judgment.meta[-1] is not None:
                title = judgment.meta[-1].get("title", judgment.doc_id)
                snippet = judgment.meta[-1].get("snippet", judgment.doc_id)
            else:
                title, snippet = judgment.doc_id, judgment.doc_id
            result.append(
                {
                    "doc_id": judgment.doc_id,
                    "doc_title": title,
                    "doc_CAL_snippet": snippet,
                    "rel": judgment.rel,
                }
            )

        return self.render_json_response(result)

