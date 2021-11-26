docker build -f Dockerfile_dev -t dev-mbta-server .

docker run -p 127.0.0.1:8888:80 -v /Users/eileen/Desktop/BU/C++/MBTA/server/controllers:/install/drogon/mbta/controllers -it dev-mbta-server:latest /bin/bash

