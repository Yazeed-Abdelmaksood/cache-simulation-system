#include <systemc.h>
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <cmath>


// FIFO Cache Policy
class FIFOCachePolicy {
public:
    using fifo_iterator = std::list<uint32_t>::const_iterator;

    FIFOCachePolicy() = default;

    ~FIFOCachePolicy() = default;

    void Insert(const uint32_t &key) {
        fifo_queue.emplace_front(key); // Used emplace_front and not push_front for better efficiency
        // ptg =+ 128;
        key_map[key] = fifo_queue.begin();
    }

    void Erase(const uint32_t &key)

    noexcept {
        auto element = key_map.find(key);
        if (element != key_map.end()) {
            fifo_queue.erase(element->second);
            key_map.erase(element);
        }
    }

    const uint32_t &KeyToRep() const

    noexcept {
        if (fifo_queue.empty()) {
            throw std::runtime_error("FIFO queue is empty, no candidate for replacement.");
        }
        return fifo_queue.back();
    }

private:
    std::list <uint32_t> fifo_queue;
    std::unordered_map <uint32_t, fifo_iterator> key_map;
};

// Memory Module
SC_MODULE(Memory) {
        sc_in < uint32_t > addr;
        sc_in<int> we; // Write enable
        sc_in<uint32_t> data_in;
        sc_out<uint32_t> data_out;

        std::unordered_map<uint32_t, uint32_t> mem;

        SC_CTOR(Memory) {
            SC_METHOD(process);
            sensitive << addr << data_in << we;
        }

        void process() {
            if (we.read() == 1) {
                // PTG : 4*32 = 128 gates
                mem[addr.read()] = data_in.read();
            } else {
                auto inMem = mem.find(addr.read());
                if (inMem != mem.end()) {
                    // PTG : 4*32 = 128 gates
                    data_out.write(inMem->second);
                } else {
                    // PTG : 4*32 = 128 gates
                    data_out.write(0); // Default value if not initialized
                }
            }
        }
};

// Cache Module
using namespace sc_core;

struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

struct Request {
    uint32_t addr;
    uint32_t data;
    int we;
};

extern "C" Result run_simulation(
        int cycles,
        int directMapped,
        unsigned cacheLines,
        unsigned cacheLineSize,
        unsigned cacheLatency,
        unsigned memoryLatency,
        size_t numRequests,
        struct Request requests[],
        const char *tracefile);


SC_MODULE(CACHE) {
        sc_in < uint32_t > data;
        sc_in<uint32_t> addr;
        sc_in<int> we;

        sc_out<bool> miss;
        sc_out<bool> hit;
        sc_out<int> primitiveGateCount;

        sc_signal<uint32_t> mem_data_out;
        sc_in<bool> clk;

        unsigned cacheLineSize;
        unsigned cacheLines;
        const int nfach = 4;
        int directMapped;
        int primitiveGateCounter = 0;
        // Saving a bit : 4 gates, Saving 32 bits : 128 = 130
        // Adding operation : 150 gates
        // Division : Subtraction + bitshift -> 150 + 128 = 280
        // Modulo : division + bitshift -> 280 + 128

        unsigned numSets;
        unsigned cacheLatency;
        unsigned memoryLatency;

        std::unordered_map<uint32_t, uint32_t> cache_data;
        FIFOCachePolicy cache_policy;

        Memory memory;

        SC_HAS_PROCESS(CACHE);

        CACHE(sc_module_name name,
        int directMapped,
        unsigned cacheLines,
        unsigned cacheLineSize,
        unsigned cacheLatency,
        unsigned memoryLatency)
        : sc_module(name), directMapped(directMapped), cacheLines(cacheLines), cacheLineSize(cacheLineSize), cacheLatency(cacheLatency), memoryLatency(memoryLatency), memory("memory")
        {
            if (directMapped) // if gleich 0 then 4-Way
            {
                // PTG : 4*32 = 128 gates

                numSets = cacheLines;
                primitiveGateCounter += 128;
            } else {
                numSets = cacheLines / nfach;
                primitiveGateCounter += 278;
            }


            memory.addr(addr);
            memory.data_in(data);
            memory.we(we);
            memory.data_out(mem_data_out);

            primitiveGateCounter += 128 * 2 + 4 + 128; // saving 4 variables

            SC_CTHREAD(behaviour, clk.pos());
        }

        void splitData(uint32_t addr, uint32_t& setIndex, uint32_t& tag)
        {
            if (directMapped) {


                setIndex = (addr / cacheLineSize) % numSets;
                primitiveGateCounter += 278 + (278 + 150); // modulo is division + subtraction
                tag = addr / cacheLineSize;
                primitiveGateCounter += 278;
            } else {
                setIndex = (addr / cacheLineSize) % numSets;
                primitiveGateCounter += 278 + (278 + 150);
                tag = addr / (cacheLineSize * numSets);
                primitiveGateCounter += 278 * 2;
            }

        }

        void behaviour() {
            while (true) {
                wait(); // Wait for the positive edge of the clock

                uint32_t setIndex, tag;
                splitData(addr.read(), setIndex, tag);

                uint32_t index = (setIndex << 16) | tag;  // Combine setIndex and tag to form the index4
                primitiveGateCounter += 4 * 16 + 32 * 4;

                if (we.read() == 1) // Write operation
                {
                    if (cache_data.find(index) != cache_data.end()) {
                        // Write hit: update the cache line and write to the memory
                        cache_data[index] = data.read();
                        primitiveGateCounter += 128;
                        hit.write(true);
                        miss.write(false);
                    } else {
                        // Write miss: write-through
                        if (cache_data.size() >= cacheLines) {
                           if(!directMapped) {
                                uint32_t keyToRemove = cache_policy.KeyToRep();
                                cache_data.erase(keyToRemove);
                                cache_policy.Erase(keyToRemove);
                            }
                           std::cout << "The cache is full" << std::endl;

                        }
                        cache_data[index] = data.read();
                        primitiveGateCounter += 128;
                        cache_policy.Insert(index);
                        primitiveGateCounter += 128;
                        hit.write(false);
                        miss.write(true);
                    }
                } else {    // Read operation
                    if (cache_data.find(index) != cache_data.end()) {
                        // Read hit: return the cached value
                        hit.write(true);
                        miss.write(false);
                    } else {
                        // Read miss: fetch from memory and load into cache
                        uint32_t fetched_data = mem_data_out.read();
                        if (cache_data.size() >= cacheLines) {
                            if(!directMapped){
                                uint32_t keyToRemove = cache_policy.KeyToRep();
                                cache_data.erase(keyToRemove);
                                primitiveGateCounter += 128;
                                cache_policy.Erase(keyToRemove);
                                primitiveGateCounter += 128;
                            }
                            std::cout << "The data cannot be found" << std::endl;

                        }
                        cache_data[index] = fetched_data;
                        primitiveGateCounter += 128;
                        cache_policy.Insert(index);
                        primitiveGateCounter += 128;
                        hit.write(false);
                        miss.write(true);
                    }
                }
                primitiveGateCount.write(primitiveGateCounter);
            }
        }
};