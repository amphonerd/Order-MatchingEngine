# Order Matching Engine (C++)

A limit order book that matches buy and sell orders, written in C++.
I built it in two versions to see how much the data structure matters.

## What it does
- Limit orders with price-time priority (best price first, then whoever came first)
- Partial fills
- Cancel by order id
- Trades happen at the price of the order that was already waiting

## Versions
- **v1_vector**: buy and sell books as sorted vectors. Simple, but slow on purpose (baseline).
- **v2_map**: each side is a `map<price, deque<Order>>`, plus an `unordered_map` from order id to (side, price) so cancel can jump straight to the right price level.

## Results
100,000 random orders, fixed seed, prices between 90 and 110, 2 runs each.

| Version | Time | Throughput |
|---|---|---|
| v1 (sorted vector) | ~43.5 s | ~2,300 orders/sec |
| v2 (map + deque + id index) | ~0.07 s | ~1.4M orders/sec |

About 600x faster. v1 takes around 45 seconds to run, so don't think it hung.

v1 re-sorts the whole vector every time an order waits and shifts elements on every erase, and this benchmark keeps thousands of orders waiting. v2 inserts in O(log P) (P = number of price levels) with no re-sorting.

## Build and run
```
g++ -std=c++14 -O2 v1_vector/v1_vector.cpp -o v1
g++ -std=c++14 -O2 v2_map/v2_map.cpp -o v2
```
Then run `./v1` or `./v2` (on Windows: `v1` or `v2`). Each prints a small demo first, then the benchmark.

## Limitations
- Single-threaded
- No order modify, no market orders
- Benchmark only uses 21 price levels and random orders, so it shows the effect of the data structure, not real exchange performance
- Cancel in v2 scans one price level, so it is O(k) for k orders at that price