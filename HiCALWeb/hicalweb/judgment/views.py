import csv
import io
import itertools
import json
import logging
import traceback

from braces import views
from django.contrib import messages
from django.http import HttpResponse
from django.http import HttpResponseRedirect
from django.http import StreamingHttpResponse
from django.urls import reverse_lazy
from django.views import generic
from interfaces.DocumentSnippetEngine import functions as DocEngine

from hicalweb.CAL.exceptions import CALError
from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.judgment.forms import UploadForm
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
            doc_title = self.request_json[u"doc_title"]
            doc_CAL_snippet = self.request_json[u"doc_CAL_snippet"]
            doc_search_snippet = self.request_json[u"doc_search_snippet"]
            relevance = self.request_json[u"relevance"]
            source = self.request_json[u"source"]
            method = self.request_json[u"method"]
            query = self.request_json.get(u"query", None)
            client_time = self.request_json.get(u"client_time", None)
            timeVerbose = self.request_json.get(u"timeVerbose")
            search_query = self.request_json[u"search_query"]
            ctrl_f_terms_input = self.request_json[u"ctrl_f_terms_input"]
            found_ctrl_f_terms_in_title = self.request_json[u"found_ctrl_f_terms_in_title"]
            found_ctrl_f_terms_in_summary = self.request_json[u"found_ctrl_f_terms_in_summary"]
            found_ctrl_f_terms_in_full_doc = self.request_json[u"found_ctrl_f_terms_in_full_doc"]
        except KeyError:
            error_dict = {u"message": u"your input must include doc_id, doc_title, "
                                      u"relevance, etc.."}

            return self.render_bad_request_response(error_dict)

        # Check if a judgment exists already, if so, update the db row.
        exists = Judgment.objects.filter(user=self.request.user,
                                         doc_id=doc_id,
                                         session=self.request.user.current_session)

        if exists:
            exists = exists.first()
            exists.query = query
            exists.doc_title = doc_title
            if doc_CAL_snippet != "":
                exists.doc_CAL_snippet = doc_CAL_snippet
            if doc_search_snippet != "":
                exists.doc_search_snippet = doc_search_snippet
            exists.relevance = relevance
            exists.source = source
            exists.method = method
            exists.timeVerbose.append(timeVerbose)
            exists.search_query = search_query
            exists.ctrl_f_terms_input = ctrl_f_terms_input
            exists.found_ctrl_f_terms_in_title = found_ctrl_f_terms_in_title
            exists.found_ctrl_f_terms_in_summary = found_ctrl_f_terms_in_summary
            exists.found_ctrl_f_terms_in_full_doc = found_ctrl_f_terms_in_full_doc
            exists.save()

        else:
            Judgment.objects.create(
                user=self.request.user,
                doc_id=doc_id,
                doc_title=doc_title,
                doc_CAL_snippet=doc_CAL_snippet,
                doc_search_snippet=doc_search_snippet,
                session=self.request.user.current_session,
                query=query,
                relevance=relevance,
                source=source,
                method=method,
                timeVerbose=[timeVerbose],
                search_query=search_query,
                ctrl_f_terms_input=ctrl_f_terms_input,
                found_ctrl_f_terms_in_title=found_ctrl_f_terms_in_title,
                found_ctrl_f_terms_in_summary=found_ctrl_f_terms_in_summary,
                found_ctrl_f_terms_in_full_doc=found_ctrl_f_terms_in_full_doc
            )

        context = {u"message": u"Your judgment on {} has been received!".format(doc_id),
                   u"is_max_judged_reached": False}
        error_message = None

        is_from_cal = source == "CAL"
        is_from_search = "search" in source
        # mark relevant documents as 1 to CAL.
        rel_CAL = -1 if relevance <= 0 else 1

        if is_from_cal:
            context[u"next_docs"] = []
            try:
                next_patch = CALFunctions.send_judgment(
                    self.request.user.current_session.uuid,
                    doc_id,
                    rel_CAL)
                if not next_patch:
                    return self.render_json_response(context)

                doc_ids_hack = []
                for doc_id in next_patch:
                    doc = {'doc_id': doc_id}
                    if '.' in doc_id:
                        doc['doc_id'], doc['para_id'] = doc_id.split('.')
                    doc_ids_hack.append(doc)

                if 'doc' in self.request.user.current_session.strategy:
                    documents = DocEngine.get_documents(next_patch,
                                                        self.request.user.current_session.topic.seed_query)
                else:
                    documents = DocEngine.get_documents_with_snippet(doc_ids_hack,
                                                                     self.request.user.current_session.topic.seed_query)
                context[u"next_docs"] = documents
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. "
                                          u"Please check status of servers."}
                return self.render_timeout_request_response(error_dict)
            except CALError as e:
                error_message = "CAL Exception: {}".format(str(e))
            except Exception as e:
                error_message = str(e)

        elif is_from_search:
            try:
                CALFunctions.send_judgment(self.request.user.current_session.uuid,
                                           doc_id,
                                           rel_CAL)
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
                "client_time": client_time,
                "result": {
                    "message": error_message,
                    "action": "create",
                    "doc_judgment": {
                        "doc_id": doc_id,
                        "doc_title": doc_title,
                        "topic_number": self.request.user.current_session.topic.number,
                        "session": str(self.request.user.current_session.uuid),
                        "query": query,
                        "relevance": relevance,
                        "source": source,
                        "method": method,
                        "timeVerbose": timeVerbose,
                        "search_query": search_query,
                        "ctrl_f_terms_input": ctrl_f_terms_input,
                        "found_ctrl_f_terms_in_title": found_ctrl_f_terms_in_title,
                        "found_ctrl_f_terms_in_summary": found_ctrl_f_terms_in_summary,
                        "found_ctrl_f_terms_in_full_doc": found_ctrl_f_terms_in_full_doc
                    }
                }
            }

            logger.error("[{}]".format(json.dumps(log_body)))

        if not exists:
            # Check if user has judged `max_judged` documents in total.
            judgements = Judgment.objects.filter(user=self.request.user,
                                                 session=self.request.user.current_session)
            max_judged = self.request.user.current_session.max_number_of_judgments
            # Exit task only if number of judgments reached max (and maxjudged is enabled)
            if len(judgements) >= max_judged > 0:
                self.request.user.current_session = None
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
            doc_title = self.request_json[u"doc_title"]
            doc_search_snippet = self.request_json[u"doc_search_snippet"]
            query = self.request_json.get(u"query", None)
            client_time = self.request_json.get(u"client_time", None)
            timeVerbose = self.request_json.get(u"timeVerbose")
        except KeyError:
            error_dict = {u"message": u"your input must include doc_id, doc_title, "
                                      u"doc_search_snippet, etc.."}
            return self.render_bad_request_response(error_dict)

        # Check if a judgment exists already, if so, update the db row.
        exists = Judgment.objects.filter(user=self.request.user,
                                         doc_id=doc_id,
                                         session=self.request.user.current_session)

        if exists:
            exists = exists.first()
            exists.query = query
            exists.doc_title = doc_title
            exists.doc_search_snippet = doc_search_snippet
            exists.timeVerbose.append(timeVerbose)
            exists.save()

        else:
            Judgment.objects.create(
                user=self.request.user,
                doc_id=doc_id,
                doc_title=doc_title,
                doc_CAL_snippet="",
                doc_search_snippet=doc_search_snippet,
                session=self.request.user.current_session,
                query=query,
                relevance=None,
                source=None,
                method=None,
                timeVerbose=[timeVerbose],
            )

        context = {u"message": u"Your no judgment on {} has been received!".format(doc_id)}

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
                    session=self.request.user.current_session,
                    source="CAL"
                 ).filter(
                    relevance__isnull=False
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
                    "relevance": judgment.relevance,
                }
            )

        return self.render_json_response(result)


class JudgmentsView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'judgment/judgments.html'

    def get_context_data(self, **kwargs):
        context = super(JudgmentsView, self).get_context_data(**kwargs)

        judgments = Judgment.objects.filter(user=self.request.user,
                                            session=self.request.user.current_session,
                                            relevance__isnull=False)

        context["judgments"] = judgments
        context['upload_form'] = UploadForm()
        return context

    def post(self, request, *args, **kwargs):
        try:
            csv_file = request.FILES['csv_file']
            train_model = request.POST.get('train_model')
            update_existing = request.POST.get('update_existing')
        except KeyError:
            messages.error(request, 'Ops! Something wrong happened. '
                                    'Could not upload judgments.')
            return HttpResponseRedirect(reverse_lazy('judgment:view'))
        if not csv_file.name.endswith('.csv'):
            messages.error(request, 'Please upload a file ending with .csv extension.')
            return HttpResponseRedirect(reverse_lazy('judgment:view'))

        train_model = train_model == "on"
        update_existing = update_existing == "on"
        try:
            data = csv_file.read().decode('UTF-8')
        except UnicodeEncodeError:
            messages.error(request, 'Ops! Something wrong happened while encoding file.')
            return HttpResponseRedirect(reverse_lazy('judgment:view'))

        try:
            io_string = io.StringIO(data)
            reader = csv.DictReader(io_string)
        except csv.Error:
            messages.error(request, 'Ops! Please make sure you upload a valid csv file.')
            return HttpResponseRedirect(reverse_lazy('judgment:view'))

        new, updated, failed = 0, 0, 0
        for row in reader:
            try:
                docno, rel = row['docno'], int(row['judgment'])
            except KeyError:
                messages.error(request, 'Ops! Please make sure you upload a valid csv file.')
                return HttpResponseRedirect(reverse_lazy('judgment:view'))

            # Check if docid is valid
            if not CALFunctions.check_docid_exists(self.request.user.current_session.uuid,
                                                   docno):
                failed += 1
                continue

            # check if judged
            judged = Judgment.objects.filter(user=self.request.user,
                                              doc_id=docno,
                                              session=self.request.user.current_session)
            if train_model:
                try:
                    CALFunctions.send_judgment(self.request.user.current_session.uuid,
                                               docno,
                                               1 if rel > 0 else -1)
                except (TimeoutError, CALError):
                    failed += 1
                    continue

            if judged.exists():
                if update_existing:
                    judged = judged.first()
                    judged_rel = judged.relevance
                    if judged_rel != rel:
                        judged.relevance = rel
                        judged.source = "uploaded"
                        judged.save()
                        updated += 1
            else:
                Judgment.objects.create(
                    user=self.request.user,
                    doc_id=docno,
                    session=self.request.user.current_session,
                    relevance=rel,
                    source="uploaded",
                )
                new += 1

        if failed:
            messages.error(request, 'Ops! {} judgments were not recorded.'.format(failed))

        messages.success(request,
                         ("Added {} new judgments. ".format(new) if new else "") +
                         ("{} judgments were updated".format(updated) if updated else ""),
                         )

        return HttpResponseRedirect(reverse_lazy('judgment:view'))

    def get(self, request, *args, **kwargs):

        if request.GET.get("export_csv"):
            class Echo:
                """An object that implements just the write method of the file-like
                interface.
                """

                def write(self, value):
                    """Write the value by returning it, instead of storing in a buffer."""
                    return value

            judgments = Judgment.objects.filter(user=self.request.user,
                                                session=self.request.user.current_session,
                                                relevance__isnull=False)
            header = ["docno", "judgment"]
            rows = ([judgment.doc_id, judgment.relevance] for judgment in judgments)
            data = itertools.chain([header], rows)
            filename = "{}.csv".format(str(self.request.user.current_session.uuid))
            pseudo_buffer = Echo()
            writer = csv.writer(pseudo_buffer)
            response = StreamingHttpResponse((writer.writerow(row) for row in data),
                                             content_type="text/csv")
            response['Content-Disposition'] = 'attachment; filename="{}"'.format(filename)
            return response
        return super(JudgmentsView, self).get(self, request, *args, **kwargs)
