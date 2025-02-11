# Histogram | CS 6300 | Nick Waddoups

The goal of this project was to write a _histogram generator_ that takes in a vector of 
floating point numbers and then generates a histogram data with a given number of bins 
containing the count of numbers in that range as well as the max value in the range.

For example:

Given input `[1, 2, 3, 4, 5]`, and directed to use two bins with ranges `(0,3], (3,6]`, 
return the following:

```
Counts: [Bin0: 3, Bin1: 2]
Maxes:  [Bin0: 3, Bin1: 5]
```

The output of this project was to be a command line program, `histogram`, that you could 
give the following command line arguments to to generate these bins. Datasets are to be 
randomly generated based on min/max measurements and the data count.

```sh
# Usage
./histogram <number of threads> <bin count> <min meas> <max meas> <data count>
```

A purpose of this program was to see the speedup that multithreading could produce, so I 
will implement three histogram generators:

- A serial histogram generator (single threaded),
- a global sum histogram generator (multi threaded),
- and a tree structed sum historgram generator (multi threaded).

## Prequisites for Building

- Unix-style system (for commands like `mkdir`)
- CMake 3.22 (or higher) installed
- C++20 / C++23 compliant compiler (tested on AppleClang 16)
- Python 3 
- Make

## Building 

1. Clone my `usu_spring_2025` repository.
2. Checkout the "hpc" branch.
3. Change into the "Homework/2" directory.
4. Type `make`

This will build three targets, `auto`, `cli`, and `test`.

- `auto`: This build generates the program I used to generate timing results for the plots. It 
  runs the Serial, Global, and Tree-structured histogram generators many times over a wide range 
  of datasizes and thread counts. NOTE: this program in it's current configuration took about 
  3 hrs and 20 minutes to run on my M3 MacBook pro with 18Gb RAM.

- `cli`: This build generates the command line histogram interface as specified above.

- `test`: This generates tests that ensure that each implementation is correctly calculating the 
  histogram data.

### Other Options 

There are several other targets in the Makefile that you can target. I won't exhaustively cover them 
here, but there are notes in the Makefile. The one I will cover is `make histogram`. This runs the 
build process and then symlinks `./build/cli` to `./histogram` for easy access to the command line  
application.

## Files 

- Plots/
    - Contains plots generated for the assignment.
- include/
    - Contains header files for the three histogram generators (called _solvers_ for some reason in 
      my code).
    - Contains other header files shared across the project.
- results/
    - Contains the output of `auto` as a `.csv`
- solvers/
    - Contains implementation details for each of the histogram generators (solvers).
- src/ 
    - Contains implementation details for shared source files (currently on the command line parser).

## Results 

Overall, this lab showed that more threads equals more faster. See [plots/](./plots/) for more 
information.

## Lessons Learned

This small project was a great opportunity to learn some about parallel programming and C++. Here
are some of my takeaways:

- Putting all of the simple types into a file called `types.h` is great until you realize that everytime 
  you change _any_ type, basicaly the entire project has to be recompiled. I think in the future it would be
  better to stick different classes in different files, even if they are just pretty short. I think that 
  grouping classes together by expected use scope is probably the way to go.
- Working with `std` algorithms and not using `for`-loops, pointers, or indices is an excellent way to write 
  code that doesn't crash or segfault, even in the edge cases. I was really impressed with how well my code 
  handled unexpected-ly sized inputs.
- Immediately evaluated lambda expressions actually are excellent, despite my original misgivings. I found 
  them so useful for replacing ternary operators and little assignments where I needed to generate an 
  intermediate value.

  ```cpp
  // Old way 
  const auto intermediate = /* some calculation */;
  int value;

  if (/* predicate */) {
    value = intermediate /* operation */;
  } else {
    value = intermediate /* other operation */;
  }
  ```

  This is the way I used to do calculations like this. It works fine, but suffers from a few issues that
  I wish it didn't. First, `intermediate` is introduced into the same scope as `value`, even if it's not
  going to be needed anywhere else. Secondly, value's type must be explicity declared and can't be `const`
  unless the `if/else` statement will need to be collapsed to a ternary operation, which could prove
  difficult if either branch is a little more complex than a single expression. Using a constantly 
  evaluated lambda can improve this significantly (in my opinion):

  ```cpp
  const auto value = [&](){
    const auto intermediate = /* some calculation */;

    if (/* predicate */) {
      return intermediate /* operation */;
    } else {
      return intermediate /* other operation */;
    }
  }(); // immediately call the lambda where it's created to get a value instead
       // of a callable
  ```

  This way, `intermediate` doesn't pollute the same scope as `value`, we can still use `const auto`, and
  with optimizations turned on the same machine code should get generated.
- Setting up simple abstractions that can be optimized away makes for really flexible algorithms and
  code. For the "tree-structured" histogram generator I used semaphores to ensure that all of the
  threads merge their calculations correctly. I wrote a simple class to abstract away the functionality
  of the semaphores in order to make the algorithm easier to implement, but it actually ended up being 
  great for another reason. After talking to my professor about using `std::binary_semaphore` instead of
  Pthread semaphores I realized that the matter of switching between the two implementations of semaphores
  was actually pretty straightforward because I had abstracted the specifics away.
