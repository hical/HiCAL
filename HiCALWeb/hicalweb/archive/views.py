import csv
import io
import itertools
import logging


from braces import views
from django.contrib import messages
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic
from django.http import StreamingHttpResponse

from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.judgment.forms import UploadForm
from hicalweb.judgment.models import Judgment
from hicalweb.CAL.exceptions import CALError

logger = logging.getLogger(__name__)


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'archive/home.html'

    def get_context_data(self, **kwargs):
        context = super(HomePageView, self).get_context_data(**kwargs)

        judgments = Judgment.objects.filter(user=self.request.user,
                                             task=self.request.user.current_task,
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
            return HttpResponseRedirect(reverse_lazy('archive:main'))
        if not csv_file.name.endswith('.csv'):
            messages.error(request, 'Please upload a file ending with .csv extension.')
            return HttpResponseRedirect(reverse_lazy('archive:main'))

        train_model = train_model == "on"
        update_existing = update_existing == "on"
        try:
            data = csv_file.read().decode('UTF-8')
        except UnicodeEncodeError:
            messages.error(request, 'Ops! Something wrong happened while encoding file.')
            return HttpResponseRedirect(reverse_lazy('archive:main'))

        try:
            io_string = io.StringIO(data)
            reader = csv.DictReader(io_string)
        except csv.Error:
            messages.error(request, 'Ops! Please make sure you upload a valid csv file.')
            return HttpResponseRedirect(reverse_lazy('archive:main'))

        new, updated, failed = 0, 0, 0
        for row in reader:
            try:
                docno, rel = row['docno'], int(row['judgment'])
            except KeyError:
                messages.error(request, 'Ops! Please make sure you upload a valid csv file.')
                return HttpResponseRedirect(reverse_lazy('archive:main'))

            # Check if docid is valid
            if not CALFunctions.check_docid_exists(self.request.user.current_task.uuid,
                                                   docno):
                failed += 1
                continue

            # check if judged
            judged = Judgment.objects.filter(user=self.request.user,
                                              doc_id=docno,
                                              task=self.request.user.current_task)
            if train_model:
                try:
                    CALFunctions.send_judgment(self.request.user.current_task.uuid,
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
                    task=self.request.user.current_task,
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

        return HttpResponseRedirect(reverse_lazy('archive:main'))

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
                                                 task=self.request.user.current_task,
                                                 relevance__isnull=False)
            header = ["docno", "judgment"]
            rows = ([judgment.doc_id, judgment.relevance] for judgment in judgments)
            data = itertools.chain([header], rows)
            filename = "{}.csv".format(str(self.request.user.current_task.uuid))
            pseudo_buffer = Echo()
            writer = csv.writer(pseudo_buffer)
            response = StreamingHttpResponse((writer.writerow(row) for row in data),
                                             content_type="text/csv")
            response['Content-Disposition'] = 'attachment; filename="{}"'.format(filename)
            return response
        return super(HomePageView, self).get(self, request, *args, **kwargs)
