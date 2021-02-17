# cpp-kanren
An implementation of mini-kanren in C++

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
