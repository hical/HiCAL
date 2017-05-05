import httplib2
import urllib

from config.settings.base import CAL_SERVER_IP, CAL_SERVER_PORT
from interfaces.DocumentSnippetEngine import functions as DocEngine

def send_judgment(session, doc_id, next_batch_size=5):
    # data = {'session': session,
    #         'doc_id': doc_id}
    # body = urllib.urlencode(data)
    # h = httplib2.Http()
    # resp, content = h.request("http://{}:{}".format(CAL_SERVER_IP, CAL_SERVER_PORT),
    #                           method="POST", body=body)

    # testing solution
    from uuid import uuid4
    doc_ids = [str(uuid4().hex[0:5]) for _ in range(next_batch_size)]
    return DocEngine.dummy_get_documents(doc_ids, "dummy")  # temp solution


# TODO: complete this function
def get_documents(session, num_docs, query):
    """
    :param session: current session
    :param num_docs: number of documents to return
    :return: return JSON list of documents_ids to judge
    """
    #  TODO: write this function


    # testing solution
    from uuid import uuid4
    doc_ids = [str(uuid4().hex[0:5]) for _ in range(num_docs)]
    return DocEngine.dummy_get_documents(doc_ids, query)  # temp solution
