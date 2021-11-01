docker build -f Dockerfile_debug -t debug-mbta-server .

docker run -p 127.0.0.1:3003:80 -v /Users/eileen/Desktop/BU/C++/MBTA/server/controllers:/install/drogon/mbta/controllers -it debug-mbta-server:latest /bin/bash
