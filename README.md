# This is the official README document for Cache Simulation Project for GRA course

## Contents
1. Introduction to the theme of Caches
2. Introduction to the classes and structure of the Project
3. Yazeed's area of work
4. Adrian's area of work
5. Samhitha's area of work
6. Problems and errors we encountered
7. Research
8. Conclusion

## 1: Caches

Caches are software or hardware temporary data memory. They are built like ay other data mmory, 
with the only difference that the Cache does not intend to store data for a longer time.

Caches have to be big, but not too big as to not slow down the process, 
but big enough to save enough data.
There is no default size, but according to industry practices, the cache's size
should be 10% of the HDD's space.
The "basic size" of the cache is usually assigned at system start.

Following are the cache sizes of some select processors and CPUs : 

| Name                   | Type                                  | L1- Size | L2 - Size | L3 - Size | Source                                                                                            |
|------------------------|---------------------------------------|----------|-----------|-----------|---------------------------------------------------------------------------------------------------| 
| AMD Ryzen 5 PRO 7640HS | Notebook Processor                    | 384 KB   | 6MB       | 16MB      | https://www.notebookcheck.com/AMD-Ryzen-5-PRO-7640HS-Prozessor-Benchmarks-und-Specs.725649.0.html |
| AMD Ryzen 7 3700       | Regular Laptop Processor              | 384 KB   | 2 MB      | 4MB       | https://www.notebookcheck.com/AMD-Ryzen-7-3700U-Prozessor.390991.0.html                           |
| AMD Ryzen 7 5700G      | Desktop-APU                           | 512 KB   | 4MB       | 16MB      | https://www.notebookcheck.com/AMD-Ryzen-7-5700G-Prozessor-Benchmarks-und-Specs.589253.0.html      |
| AMD Ryzen 9 5900 HX    | Mobile Processor for bigger computers | 512 KB   | 4MB       | 16MB      | https://www.notebookcheck.com/AMD-Ryzen-9-5900HX-Prozessor-Benchmarks-und-Specs.512760.0.html     |
| Intel Core i5-6287U    | Laptop Processor                      | 128 KB   | 512 KB    | 4MB       | https://www.notebookcheck.com/Intel-Core-i5-6287U-Notebook-Prozessor.149426.0.html                |
| Intel Core i7-10510U   | Laptop Processor                      | 256 KB   | 1MB       | 8MB       | https://www.notebookcheck.com/Intel-Core-i7-10510U-Notebook-Prozessor-Comet-Lake.430494.0.html    |
| Intel Core i9-14900K   | High-End Processor                    | 256 KB   | 1MB       | 8MB       | https://www.notebookcheck.com/Intel-Core-i9-14900K-Prozessor-Benchmarks-und-Specs.759078.0.html   |

The size of caches in modern Processors is slowly growing with the size of data memories; according to Moore's Law.


## 2: Classes and structures

The cache simulation uses the main.c class as a frame programme. Here, the transferred parameters and input file are read and converted into a request array.
From there, the call to start the simulation is made in the sc_main.cpp class.
This initializes the SystemC modules that simulate the cache with the given parameters.
Our cache's replacement structure in case of cache hits is the FIFO Policy. We push the first item entered out of the structure, load a new cache line from the main memoryy, and push it into our cache.
Talking about the main memory, it is a model and mirrors our cache's structure. It is written to in case there is any change in the cache, and read from in case of a replacement.

The data is saved in form of a std::unordered hash map. This helps us because the insert and remove operations are O(1). 
We take in the request array and work through the array one by one. Each parameter is passed to the cache module in form of input signals, and handled accordingly in the module.
While handling the requests and inserting or reading them from the cache, we count the amount of cycles the whole operation took, 
record a cache miss or a cache hit, and the primitive gate count. This is all recorded in the Result struct. 
At last, the programme returns the Result struct back to the C-Part in main.c, which displays the information on the console. 



## example csv file :
W;0xAB1;0x42
W;0xABA;0x13
W;0xAC3;0x17
W;0xACD;0x83
W;0xAD5;0x49
R;0xAB1;
R;0xAB2;
W;0xAD6;0x23

# 3: Yazeed

- **sc_main / C++ Function**  
  Sets up the SystemC simulation environment, including module initialization, connections, and simulation start based on user input.

- **4-Way Set-Associative Cache**  
  Implements a 4-way set-associative cache using `unordered_map` for efficient data access and `list` for managing cache lines.

- **Memory Module**  
  Simulates basic memory operations with `unordered_map` for efficient access, handling read and write operations.

- **FIFO Cache Replacement Policy**  
  Manages cache data with a FIFO (First-In-First-Out) policy to determine which cache line to evict.

- **Cache and Memory Latency Simulation**  
  Models realistic memory access times by simulating both cache and memory latencies.

# 4 : Samhitha

- README, Research about cache sizes and general project information 
- Slides

# 5 : Adrian

- Implementation of the complete C program
- Research about sizes of modern caches
- Research about latencies of modern caches
- Research about SelectionSort
- Creation of the example csv files

# 6 : Problems

1. Implementation of a Hash Map in SystemC and handling sc_phash, we settled for using the std library version
2. Implementation of the write-through cache and creating and attaching the memory module
3. Differentiating the implementation between a direct-mapped and 4-way associative cache

# 7 : Research

1. Size and Latencies of modern Caches
The size and latency of modern caches depend heavily on their level. Level 1 caches often require only a few cycles. This also depends on whether complex addresses or simple pointers need to be read. Many modern processors also have two separate Level 1 caches, each serving a different function. The Level 2 cache is significantly larger than the Level 1 cache but often requires a longer access time. However, the access time is still short enough so that frequent accesses do not cause significant delays. The Level 3 cache is often shared among the various cores of the processor. The access time for this cache is relatively high, but due to its size, it can retain a lot of data. The following tables list the various latencies and sizes of caches in different microarchitectures.

Cache Sizes per Core

| Cache Level | Coffee Lake | Skylake Server | Zen 2    |
|-------------|-------------|----------------|----------|
| Level 1     | 2x32 KiB    | 2x32 KiB       | 2x32 KiB |
| Level 2     | 256 KiB     | 1 MiB          | 512 KiB  |
| Level 3     | 1 MiB       | 1.375 MiB      | 1 MiB    |

Cache Latencies (in cycles)

| Cache Level | Coffee Lake | Skylake Server | Zen 2        |
|-------------|-------------|----------------|--------------|
| Level 1     | 4/5         | 4/5            | 4-8          |
| Level 2     | 12          | 14             | 12-17        |
| Level 3     | 42          | 50-70          | 40 (average) |

The latency of main memory in modern processors is about 200 cycles, but can be lower or higher.

2. SelectionSort
Many Sorting algorithms that sort in-place can profit from caches. We have chosen SelectionSort as a fitting example.
SelectionSort is a simple sorting algorithm that sorts a list by repeatedly finding and placing the smallest (or largest) element. A significant advantage of SelectionSort with respect to caches is its sequential memory access pattern. The algorithm traverses the array multiple times and accesses adjacent memory cells. This way, the algorithm can optimally use the spatial locality of caches. This reduces the accesses to the main memory and accelerates the execution of the algorithm.
Examples of the SelectionSort memory access pattern can be found in the examples folder.

3. Data structure used in implementing Caches
Design Decisions:
• List vs. Array: Initially considered using arrays for their simplicity, but opted for Lists instead. Arrays could lead to issues such as segmentation faults and lack flexibility, while Lists provide more robust and safer data management.

• Efficiency with unordered_map: To ensure efficient access and management of cache data, std::unordered_map was used. It offers average constant-time complexity for lookups, insertions, and deletions, making it ideal for handling cache operations effectively.
    • To understand the use of `unordered_map`, we referred to a simple example from GeeksForGeeks. This example illustrates fundamental operations such as initialization, indexing, and iteration through an `unordered_map`:

```cpp
int main() {
    // Declaration of an unordered_map with <string, double> as the types for key and value
    unordered_map<string, double> umap = {
        {"One", 1},
        {"Two", 2},
    };

    // Inserting values using the [] operator
    umap["PI"] = 3.14;
    umap["root2"] = 1.414;
    string key = "PI";
    if (umap.find(key) == umap.end())
        cout << key << " not found\n\n";
    else
        cout << "Found " << key << "\n\n";

    // Iterating over all elements of the unordered_map
    unordered_map<string, double>::iterator itr;
    cout << "\nAll Elements : \n";
    for (itr = umap.begin(); itr != umap.end(); itr++)
        cout << itr->first << " " << itr->second << endl;

    return 0;
}
```

# 9 : Conclusion and further thoughts

The project taught us a lot about caches and elementary operations that happen in computers that is often unseen to the user.

Our Cache implementation is just a model implementation. What could be done in the future/further are the following : 
- Optimization. We can try to parallely implement the cache, and have multiple threads deal with the insertion and removing of data. We would have to use semaphores and mutexes in our code to ensure there is no deadlock or overwriting.
- Using a more real-life useful replacement policy. While FIFO is good and easy to comprehend, using a policy that actually works with how caches are used in real life (such as Least Frequently Used) will help make the implementation more realistic.






