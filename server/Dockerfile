FROM drogonframework/drogon:latest

RUN drogon_ctl create project mbta

COPY controllers /opt/app/controllers

RUN cd /install/drogon/mbta && \
    cp /opt/app/controllers/* controllers/ && \
    cd build && \
    cmake .. && \
    make

CMD ["/install/drogon/mbta/build/mbta"]

