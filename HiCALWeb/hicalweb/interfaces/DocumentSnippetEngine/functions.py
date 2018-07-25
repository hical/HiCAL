from config.settings.base import DOCUMENTS_URL
from config.settings.base import PARA_URL

import httplib2

try:
    # For c speedups
    from simplejson import loads
except ImportError:
    from json import loads


def get_date(content):
    for line in content.split('\n'):
        if line.strip()[:4] == "Sent":
            return line.split(':', 1)[1].strip()
    return ""


def get_subject(content):
    for line in content.split('\n'):
        if line.strip()[:7] == "Subject":
            return line.split(':', 1)[1].strip()
    return ""


def get_documents(doc_ids, query=None):
    """
    :param query:
    :param doc_ids: the ids of documents to return
    :return: documents content
    """
    result = []
    h = httplib2.Http()
    for doc_id in doc_ids:
        url = '{}/{}'.format(DOCUMENTS_URL, doc_id)
        resp, content = h.request(url.format(DOCUMENTS_URL, doc_id),
                                  method="GET")
        content = content.decode('utf-8', 'ignore')
        date = get_date(content)
        title = get_subject(content)
        if len(content) == 0:
            if len(title) == 0:
                title = '<i class="text-warning">The document title is empty</i>'
            content = '<i class="text-warning">The document content is empty</i>'
        else:
            if len(title) == 0:
                title = content[:32]

        document = {
            'doc_id': doc_id,
            'title': title,
            'content': content.replace("\n", "<br/>"),
            'date': date
        }
        result.append(document)

    return result


def get_documents_with_snippet(doc_ids, query):
    h = httplib2.Http()
    url = "{}/{}"
    doc_ids_unique = []
    doc_ids_set = set()
    for doc_id in doc_ids:
        if doc_id['doc_id'] not in doc_ids_set:
            doc_ids_set.add(doc_id['doc_id'])
            doc_ids_unique.append(doc_id)

    doc_ids = doc_ids_unique

    result = get_documents([doc['doc_id'] for doc in doc_ids], query)
    for doc_para_id, doc in zip(doc_ids, result):
        if 'para_id' not in doc_para_id:
            doc['snippet'] = u'N/A'
            continue
        try:
            para_id = doc_para_id['doc_id'] + '.' + doc_para_id['para_id']
            resp, content = h.request(url.format(PARA_URL, para_id),
                                      method="GET")
            doc['snippet'] = content.decode('utf-8', 'ignore').replace("\n", "<br />");
        except:
            doc['snippet'] = u'N/A'
    return result
