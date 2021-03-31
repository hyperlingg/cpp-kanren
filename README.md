# cpp-kanren
An implementation of mini-kanren using modern C++

# Build instructions
Prerequisites are
* `gcc` version 11.0.0 (experimental)
* `CMake` version 3.0 or larger

If you want to run tests
* `Boost` version 1.71 or larger

# gcc-11 source build
Needs to be built from source for most platforms:
https://gcc.gnu.org/install/index.html

Warning: takes very very long (hours) and may fail (after hours...) because of some prerequisites missing etc. . I could also demonstrate that the tests run successfully via screen sharing if you'd like to save some time.
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
All tests (except the last one) run successfully.

