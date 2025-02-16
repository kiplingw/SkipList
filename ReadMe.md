# Kip's Skip List Implementation
This is my C++17 implementation of the skip list data structure. It was first discovered by William Pugh (1989). It can be used in place of balanced trees and uses probabilistic balancing. It works well if the elements are inserted in random order, or, unlike a binary tree, in sorted order too. It has the same asymptotic expected time bounds as a binary tree, but is simpler to implement, faster (with caveats), and uses less storage.

Experimentation demonstrates that performance is not quite as Pugh had hoped. But then when he wrote his seminal paper the size of the data structure likely would not have contained billions of elements. Modern CPU caches also didn't exist either.

You can read the Pugh's original paper [here](https://dl.acm.org/doi/10.1145/78973.78977).

## Compiling / Running

This is a header only library, but a simple unit test is available. To compile and run it, execute the following:

```bash
$ g++ Test.cpp -o Test -Wall -Werror -O3 -g3 && ./Test
```

