# cjson

stb-style header only library for parsing json.

# Usage

You only need to copy `json.h` to your project. It acts as a header file. But you need to do the following in ONE of your translation units:
```c
#ifdef JSON_IMPLEMENTATION
#include "json.h"
```
