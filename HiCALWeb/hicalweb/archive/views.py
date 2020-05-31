import csv
import io
import logging

from braces import views
from django.contrib import messages
from django.db.models import Q
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from django.views import generic

from hicalweb.interfaces.CAL import functions as CALFunctions
from hicalweb.judgment.forms import UploadForm
from hicalweb.judgment.models import Judgement
from hicalweb.CAL.exceptions import CALError

logger = logging.getLogger(__name__)


class HomePageView(views.LoginRequiredMixin, generic.TemplateView):
    template_name = 'archive/home.html'

    def get_context_data(self, **kwargs):
        context = super(HomePageView, self).get_context_data(**kwargs)

        judgments = Judgement.objects.filter(Q(user=self.request.user,
                                             task=self.request.user.current_task) &
                                           (
                                               Q(highlyRelevant=True) |
                                               Q(relevant=True) |
                                               Q(nonrelevant=True)
                                           ))
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
                docno, rel = row['docno'], row['judgment'].lower()
            except KeyError:
                messages.error(request, 'Ops! Please make sure you upload a valid csv file.')
                return HttpResponseRedirect(reverse_lazy('archive:main'))
            rel = 1 if rel == "relevant" else -1 if rel == "nonrelevant" else 2
            # Check if docid is valid
            if not CALFunctions.check_docid_exists(self.request.user.current_task.uuid,
                                                   docno):
                failed += 1
                continue

            # check if judged
            judged = Judgement.objects.filter(user=self.request.user,
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
                    judged_rel = 2 if judged.highlyRelevant else 1 if judged.relevant else -1
                    if judged_rel != rel:
                        judged.highlyRelevant = rel == 2
                        judged.relevant = rel == 1
                        judged.nonrelevant = rel == -1
                        judged.source = "uploaded"
                        judged.save()
                        updated += 1
            else:
                Judgement.objects.create(
                    user=self.request.user,
                    doc_id=docno,
                    task=self.request.user.current_task,
                    highlyRelevant=rel == 2,
                    relevant=rel == 1,
                    nonrelevant=rel == -1,
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
        return super(HomePageView, self).get(self, request, *args, **kwargs)
