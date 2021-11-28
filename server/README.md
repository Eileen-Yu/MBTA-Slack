Run server
```shell
docker build -t mbta-server .
docker run -p 127.0.0.1:3003:80 mbta-server:latest
```

Dev
```shell
sh dev.sh

# In the container
cd build
cmake ..
make
./mbta
```

