# SCHEME INTERPRETER

## Build Instructions
The build process should be relatively straight foreward, thanks to the included CMake instructions.
I used a Macbook running macOS Catalina 10.15.6 during the development of this interpreter.
Thus I cannot vouch for any other operating systems but I don't see why it shouldn't work :)

```bash
> cd <repo>
> mkdir build
> cd build
> cmake ..
> make
> ./scheme
```

Please also note that this was my first time writing anything substantial in C++. Weird language, especially when coming from python. Nonetheless, this was quite fun but in equal measures also frustrating. Well worth it though!

## Design Philosophy

* **free functions** over class functions
  * I generally like to stay as close to functional programming as possible - in this case meaning that I use classes and structs exclusively as data containers / data types. Relevant functionality is provided by free (friend) functions instead and lambdas are made use of where sensible.
* **readability** over performance
  * in the end, I don't care too much about how performant my solution is - if there's need for it, this can be done later. In the meantime, I'm going to gain a lot more from having code that others (and future me) can understand.
* **regex** based parser
  * I've got a faible for regular expressions, so I thought this was fitting. Probably not the most performant of all options, but readable and concise.

## Functionality

* all basic types
  * int
  * float
  * string
  * booleans
  * nil
* builtin functions
* custom functions
* tail call optimization
* garbage collection

## Open Issues