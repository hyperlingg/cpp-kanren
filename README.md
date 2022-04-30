# cpp-kanren
An implementation of mini-kanren using C++20 coroutines.

# Build instructions
Prerequisites are
* `gcc` version 11.0.0
* `CMake` version 3.0 or larger

For testing
* `Boost` version 1.71 or larger

# gcc-11 source build
Needs to be built from source for most platforms:
https://gcc.gnu.org/install/index.html
## configuration parameter
../gcc-10/configure --disable-multilib

## A `libstdc++` problem that occured 
If gcc-11 is installed in the default directory /usr/local then it will use an outdated libstdc++.
Setting the load library path new helps, e.g.:
```
export LD_LIBRARY_PATH=/usr/local/lib64:$LD_LIBRARY_PATH
```

# minikanren build
```
mkdir build
cd build
cmake ../
cmake --build .
```

## build and run testcases
```
cd test
mkdir test_build
cd test_build
cmake ../
cmake --build .
```

