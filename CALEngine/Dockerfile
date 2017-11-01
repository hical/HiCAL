FROM ubuntu:14.04

RUN apt-get update -y && apt-get install -y \
    libfcgi-dev spawn-fcgi gcc libarchive-dev make git

VOLUME ["/data"]
COPY . /data/CALEngine

WORKDIR "/data/CALEngine"
RUN tar xvzf data.tar.gz && rm data.tar.gz
RUN make bmi_fcgi

CMD ["/usr/bin/spawn-fcgi", "-p", "8002", "-n", "--", "/data/CALEngine/bmi_fcgi", \
    "--doc-features", "/data/CALEngine/athome1.bin", \
    "--df", "/data/CALEngine/athome1.df", "--async-mode", "--threads", "8"]

# ENV FCGI_VERSION 2.10-26

# RUN mkdir -p /tmp/build_fastcgi && cd /tmp/build_fastcgi && \
#     git clone https://github.com/lmovsesjan/Fastcgi-Daemon.git && \
#     cd Fastcgi-Daemon && \
#     dpkg-buildpackage -rfakeroot && \
#     cd .. && \
#     sudo dpkg -i \
#         ./fastcgi-daemon2-init_${FCGI_VERSION}_amd64.deb \
#         ./fastcgi-daemon2_${FCGI_VERSION}_amd64.deb \
#         ./libfastcgi-daemon2-dev_${FCGI_VERSION}_amd64.deb \
#         ./libfastcgi-daemon2_${FCGI_VERSION}_amd64.deb \
#         ./libfastcgi2-syslog_${FCGI_VERSION}_amd64.deb \
#     && cd /tmp && rm -rf /tmp/build_fastcgi

# RUN apt-get install -y mongodb-dev libmongo-client-dev libboost-filesystem-dev

# VOLUME ["/data", "/tmp/fastcgi"]
# WORKDIR "/data"
# CMD ["fastcgi-daemon2", "--config=fastcgi.conf"]
