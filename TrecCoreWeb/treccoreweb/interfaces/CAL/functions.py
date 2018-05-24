from config.settings.base import CAL_SERVER_IP
from config.settings.base import CAL_SERVER_PORT
import json
import logging
import urllib.parse

import httplib2

from treccoreweb.CAL.exceptions import CALServerError

logger = logging.getLogger(__name__)


def send_judgment(session, doc_id, rel, next_batch_size=5):
    h = httplib2.Http()
    url = "http://{}:{}/CAL/judge"

    body = {'session_id': str(session),
            'doc_id': doc_id,
            'rel': rel}
    body = urllib.parse.urlencode(body)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=body,
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")

    if resp and resp['status'] == '200':
        content = json.loads(content.decode('utf-8'))
        return content['docs']
    else:
        raise CALServerError(resp['status'])


def add_session(session, seed_query):
    """
    Adds session to CAL backend server
    :param session:
    :param seed_query
    """
    h = httplib2.Http()
    url = "http://{}:{}/CAL/begin"

    body = {'session_id': str(session),
            'seed_query': seed_query,
            'judgments_per_iteration': 1,
            'async': True,
            'mode': 'para'}
    post_body = '&'.join('%s=%s' % (k, v) for k, v in body.items())

    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=post_body,
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")
    if resp and resp['status'] != '200':
        return False
    else:
        raise CALServerError(resp['status'])


def get_documents(session, num_docs, query):
    """
    :param session: current session
    :param num_docs: number of documents to return
    :return: return JSON list of documents_ids to judge
    """
    h = httplib2.Http()
    url = "http://{}:{}/CAL/get_docs?"

    parameters = {'session_id': str(session),
                  'max_count': 10}
    parameters = urllib.parse.urlencode(parameters)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT) + parameters,
                              method="GET")

    if resp and resp['status'] == '200':
        content = json.loads(content.decode('utf-8'))
        return content['docs'], content['top-terms']
    else:
        raise CALServerError(resp['status'])
