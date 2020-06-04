FROM python:3.6

ENV PYTHONUNBUFFERED 1
RUN mkdir -p /app

# Requirements have to be pulled and installed here, otherwise caching won't work
COPY ./requirements /requirements
RUN pip install -r /requirements/local.txt

COPY ./compose/production/django/entrypoint.sh /entrypoint.sh
RUN sed -i 's/\r//' /entrypoint.sh
RUN chmod +x /entrypoint.sh

COPY ./compose/local/django/start.sh /start.sh
RUN sed -i 's/\r//' /start.sh
RUN chmod +x /start.sh

### UNCOMMENT BELOW FOR PYSERINI 0.8.1.0 SUPPORT ###
#RUN echo $JAVA_HOME
#RUN apt update -y
##RUN apt install -y software-properties-common python-software-properties
#RUN apt install -y default-jdk
#RUN JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/bin/java"
#RUN export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/bin/java"

#RUN pip install pyserini==0.8.1.0
#RUN printenv

WORKDIR /app

ENTRYPOINT ["/entrypoint.sh"]
