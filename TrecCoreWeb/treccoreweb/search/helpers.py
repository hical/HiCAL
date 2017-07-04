from treccoreweb.judgment.models import Judgement


def join_judgments(document_values, document_ids, user, task):
    """
    Adds the relevance judgment of the document to the document_values dict.
    If document has not been judged yet, `isJudged` will be False.
    :param user:
    :param task:
    :param document_values:
    :param document_ids:
    :return: document_values with extra information about the document
    """

    judged_docs = Judgement.objects.filter(user=user,
                                           task=task,
                                           doc_id__in=document_ids)
    judged_docs = {j.doc_id: j for j in judged_docs}
    for key in document_values:
        isJudged = True if key in judged_docs else False
        if isJudged:
            judgedment = judged_docs.get(key)
            document_values[key]['relevance_judgment'] = {
                "highlyrelevant": judgedment.highlyrelevant,
                "nonrelevant": judgedment.nonrelevant,
                "relevant": judgedment.relevant,
            }
        document_values[key]['isJudged'] = isJudged
    return document_values


def padder(doc_ids):
    """
    Pads zeros to ids to be length of 7
    :param doc_ids: list of document ids
    :return: doc ids padded with 0 to be length of 7.
    """

    for idx, doc_id in enumerate(doc_ids):
        doc_id = doc_id.zfill(7)
        doc_ids[idx] = doc_id

    return doc_ids
