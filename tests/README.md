generate the xocde project for unit tests via
```
meson setup build --backend=xcode
```

```
docker build -t "valgrind:1.0"

docker run --volume=/Users/ian/dev:/home/src --workdir=/home/src --restart=no -it valgrind:1.0 
```