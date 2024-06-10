# Unit tests

## Building for host
Generally you want to run unit tests on the host machine so that you can debug easily.

For macos, generate the xcode project for unit tests via:
```
meson setup build --backend=xcode
```

## If you want to run valgrind in macos, use a docker file as follows:

```
docker build -t "valgrind:1.0"

docker run --volume=/Users/ian/dev:/home/src --workdir=/home/src --restart=no -it valgrind:1.0 
```

## Cross compiling for PS2

To compile and run on the PS2, use the makefile.

Ideally you could use the following, but I've not got it working yet.
```
meson setup --cross-file ps2_cross.txt buildps2 
```
