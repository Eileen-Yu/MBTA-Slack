# Server Image for Production
# Can be built as:
# docker build -t mbta-server .
# then run by:
# docker run -p 127.0.0.1:3000:80 mbta-server:latest

FROM drogonframework/drogon:latest

COPY controllers /opt/app/controllers
COPY CMakeLists.txt /opt/app/CMakeLists.txt

RUN drogon_ctl create project mbta && \
    cd mbta && \
    apt update && \
    apt install vim -y && \
    cp /opt/app/CMakeLists.txt . && \
    apt install libcurl4-openssl-dev -y && \
    cp /opt/app/controllers/* controllers/ && \
    cd build && \
    cmake .. && \
    make

WORKDIR /install/drogon/mbta/build

CMD ["./mbta"]

