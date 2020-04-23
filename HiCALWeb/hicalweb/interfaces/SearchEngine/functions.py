from collections import OrderedDict
import requests


def get_documents(query, start=0, numdisplay=20):
    """

    :param query:
    :param start:
    :param numdisplay:
    :return:
    """
    url = "http://solr:8983/solr/hical/select?defType=edismax&highlightMultiTerm=true&hl.fl=content&hl.simple.post=%3C%2Fb%3E%3C%2Fi%3E&hl.simple.pre=%3Cb%3E%3Ci%3E&hl=on&q=" + query + "&qf=content%20title&stopwords=true&usePhraseHighLighter=true&rows="+str(numdisplay)

    response = requests.get(url)

    if response.status_code == 200:
        doc_ids = []
        result = OrderedDict()
        json = response.json()
        for idx, doc in enumerate(json["response"]["docs"]):
            docno = doc["id"]
            parsed_doc = {
                "rank": idx,
                "docno": docno,
                "title": doc["title"],
                "snippet": json["highlighting"][docno]["content"][0]
            }
            result[docno] = parsed_doc
            doc_ids.append(docno)

        return result, doc_ids, 1

    return None, None, None
