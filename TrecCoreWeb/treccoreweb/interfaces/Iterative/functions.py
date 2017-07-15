from config.latinSquare.HTNISTTopicsTop60 import HTSampledTopics


def get_documents(topic_id):
    """
    :param topic_id:
    :return: list of document ids
    """
    try:
        return HTSampledTopics.get(topic_id)
    except:
        return []
