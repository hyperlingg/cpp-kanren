# cpp-kanren
An implementation of mini-kanren in C++

# Build instructions
Preliminaries are
* `gcc` version 9.3.0 (or at least full support of C++ 17)
* `CMake` version 3.0 or larger

If you want to run tests
* `Boost` version 1.71 or larger

## Basic build
```
mkdir build
cd build
cmake ../
cmake --build .
```

## Build and run tests
```
cd test
mkdir test_build
cd test_build
cmake ../
cmake --build .
```

# design draft

## fresh variables
### type
```
using variable = shared_ptr<std::string>
```
### construction of a variable "var"
```
make_shared<std::string>("var");
```
### rationale
since pointers refer to a space in memory rather than a value, they
allow us to distinguish variables with the same name. Using `shared_ptr` of C++
ensures the absence of `new`/`delete` related memory leaks.

# gcc-11 source build
## configuration
../gcc-10/configure --disable-multilib

## `libstdc++` problems
If gcc-11 is installed in the default directory /usr/local then it will use an outdated libstdc++.
Setting the load library path new helps, e.g.:
```
export LD_LIBRARY_PATH=/usr/local/lib64:$LD_LIBRARY_PATH
```
