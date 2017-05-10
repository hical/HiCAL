import httplib2
import urllib.parse
import json

from config.settings.base import CAL_SERVER_IP, CAL_SERVER_PORT
from interfaces.DocumentSnippetEngine import functions as DocEngine


def send_judgment(session, doc_id, next_batch_size=5):
    h = httplib2.Http()
    url = "http://{}:{}/CAL/judge"

    body = {'session_id': str(session),
            'doc_id': doc_id}
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=json.dumps(body),
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")
    print(resp)

    # testing solution
    from uuid import uuid4
    doc_ids = [str(uuid4().hex[0:5]) for _ in range(next_batch_size)]
    return DocEngine.dummy_get_documents(doc_ids, "Batch from " + doc_id)  # temp solution


def add_session(session, seed_query):
    """
    Adds session to CAL backend server
    :param session:
    :param seed_query
    :return:
    """
    h = httplib2.Http()
    url = "http://{}:{}/CAL/begin"

    body = {'session_id': str(session),
            'seed_query': seed_query}
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=json.dumps(body),
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")
    print(resp)
    print(json.dumps(body))


# TODO: complete this function
def get_documents(session, num_docs, query):
    """
    :param session: current session
    :param num_docs: number of documents to return
    :return: return JSON list of documents_ids to judge
    """
    #  TODO: write this function
    h = httplib2.Http()
    url = "http://{}:{}/CAL/get_docs"

    parameters = {'session_id': str(session), 'max_count': 5}
    parameters = urllib.parse.urlencode(parameters)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT,
                                         parameters),
                              method="GET")

    print(resp)

    # testing solution
    from uuid import uuid4
    doc_ids = [str(uuid4().hex[0:5]) for _ in range(num_docs)]
    return DocEngine.dummy_get_documents(doc_ids, query)  # temp solution
