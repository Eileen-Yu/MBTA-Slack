docker build -f Dockerfile_debug -t debug-mbta-server .

docker run -p 127.0.0.1:8888:80 -v /Users/eileen/Desktop/BU/C++/MBTA/server/controllers:/install/drogon/mbta/controllers -it debug-mbta-server:latest /bin/bash
