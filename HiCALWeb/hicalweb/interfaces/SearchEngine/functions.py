from collections import OrderedDict
from config.settings.base import SEARCH_SERVER_IP
from config.settings.base import SEARCH_SERVER_PORT
import urllib.parse

import httplib2
import xmltodict


def get_documents(query, start=0, numdisplay=20):
    """

    :param query:
    :param start:
    :param numdisplay:
    :return:
    """
    h = httplib2.Http()
    url = "http://{}:{}/treccore/websearchapi/search.php?{}"

    parameters = {'start': start, 'numdisplay': numdisplay, 'query': query}
    parameters = urllib.parse.urlencode(parameters)
    resp, content = h.request(url.format(SEARCH_SERVER_IP,
                                         SEARCH_SERVER_PORT,
                                         parameters),
                              method="GET")

    if resp and resp.get("status") == "200":
        xmlDict = xmltodict.parse(content)
        try:
            xmlResult = xmlDict['search-response']['results']['result']
        except TypeError:
            return None, None, None

        doc_ids = []
        result = OrderedDict()

        if not isinstance(xmlResult, list):
            xmlResult = [xmlResult]

        for doc in xmlResult:
            docno = doc["docno"].zfill(7)
            parsed_doc = {
                "rank": doc["rank"],
                "docno": docno,
                "title": doc["title"],
                "snippet": doc["snippet"]
            }
            result[docno] = parsed_doc
            doc_ids.append(docno)

        return result, doc_ids,  xmlDict['search-response']['total-time']

    return None, None, None
