from django.db.models import Q

from hicalweb.judgment.models import Judgement


def remove_judged_docs(document_ids, user, task):
    """
    Remove already judged documents for a list of document ids
    :param user:
    :param task:
    :param document_ids:
    :return: document_ids list of documents that are not judged, in the same given order.
    """

    judged_docs = Judgement.objects.filter(Q(user=user,
                                             task=task,
                                             doc_id__in=document_ids) &
                                           (
                                               Q(highlyRelevant=True) |
                                               Q(relevant=True) |
                                               Q(nonrelevant=True)
                                           )
                                           ).values_list('doc_id', flat=True)

    document_ids = [elem for elem in document_ids if elem not in judged_docs]
    return document_ids
