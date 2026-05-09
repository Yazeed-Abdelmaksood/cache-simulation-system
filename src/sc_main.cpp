#include <systemc>

#include "cache.hpp"

struct Result run_simulation(int cycles,
                             int directMapped,
                             unsigned cacheLines,
                             unsigned cacheLineSize,
                             unsigned cacheLatency,
                             unsigned memoryLatency,
                             size_t numRequests,
                             Request requests[],
                             const char *tracefile) 
                             {
    // Create signals
    sc_clock clk("clk", sc_time(1, SC_NS));
    sc_signal<uint32_t> data;
    sc_signal<uint32_t> addr;
    sc_signal<int> we;
    
    sc_signal<bool> miss;
    sc_signal<bool> hit;
    sc_signal<int> primitiveGateCount;


    // Create and bind the Cache Instance
    CACHE cache("cache", directMapped, cacheLines, cacheLineSize, cacheLatency, memoryLatency);
    cache.data(data);
    cache.addr(addr);
    cache.we(we);
    cache.miss(miss);
    cache.hit(hit);
    cache.clk(clk);
    cache.primitiveGateCount(primitiveGateCount);

    // Create and configure the trace file
    sc_trace_file* trace = nullptr;
    if (tracefile != NULL && tracefile[0] != '\0') {
        trace = sc_create_vcd_trace_file(tracefile);
        sc_trace(trace, data, "data");
        sc_trace(trace, addr, "addr");
        sc_trace(trace, we, "we");
        sc_trace(trace, hit, "hit");
        sc_trace(trace, miss, "miss");
        sc_trace(trace, primitiveGateCount, "primitiveGateCount");
    }

    // Initialize simulation
    sc_start(1, SC_NS);

    Result result = {0, 0, 0, 0};
    size_t cycleCount = 0;

    // Simulation loop
    size_t i = 0;
    for (; i < numRequests && cycleCount < cycles; ++i) {
        Request &req = requests[i];
        data.write(req.data);
        addr.write(req.addr);
        we.write(req.we);

        sc_start(1, SC_NS); // Advance simulation time

        // Read miss and hit after advancing simulation time
        if (hit.read() == 1) 
        {
            result.hits++;

        } else {
            result.misses++;
        }

        if (we || !(hit.read()))
        {
            cycleCount += (cacheLatency + memoryLatency);
        }
        else
        {
            cycleCount += (cacheLatency);
        }
        cycleCount++;
    }
    
    // If not all requests could be processed within the given cycles
    if (cycleCount >= cycles && i < numRequests) {
        result.cycles = SIZE_MAX;
    } else {
        result.cycles = cycleCount;
    }
    result.primitiveGateCount = primitiveGateCount.read();

    if (trace) {
        sc_close_vcd_trace_file(trace);
    }

    return result;
}

int sc_main(int argc, char *argv[])
{
    std::cout << "Hello " << std::endl;
    exit(1);
}
