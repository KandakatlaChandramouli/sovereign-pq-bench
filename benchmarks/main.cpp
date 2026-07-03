#include <benchmark/benchmark.h>

static void BM_EmptyLoop(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(BM_EmptyLoop);

BENCHMARK_MAIN();
