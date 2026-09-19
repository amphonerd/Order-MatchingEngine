// Order Matching Engine - v2 (map + deque)
// Build: g++ -O2 -std=c++14 v2_map.cpp -o v2 && ./v2

#include <iostream>
#include <map>
#include <deque>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;

struct Order
{
    int id;
    int Price;
    int qty;
    bool isBuy;
};

bool printTrades = true; // turn off for the benchmark, printing is slow

map<int, deque<Order>> Sell;              // price -> orders, lowest price first
map<int, deque<Order>, greater<int>> Buy; // price -> orders, highest price first

// id -> (isBuy, price), so cancel doesn't have to search everything
unordered_map<int, pair<bool, int>> location;

void addBuyOrder(Order o)
{
    // keep matching while a cheap enough seller exists
    while (o.qty > 0 && !Sell.empty() && Sell.begin()->first <= o.Price)
    {
        auto &level = Sell.begin()->second;
        Order &resting = level.front();     // whoever came first at that price

        int traded = min(o.qty, resting.qty);
        o.qty -= traded;
        resting.qty -= traded;

        
        if (printTrades)
            cout << "Trade: Buyer " << o.id << " with Seller " << resting.id
                 << " | Qty: " << traded << " | Price: " << Sell.begin()->first << endl;

        if (resting.qty == 0)
        {
            location.erase(resting.id);
            level.pop_front();
            if (level.empty())
                Sell.erase(Sell.begin());
        }
    }
    if (o.qty > 0)
    {
        Buy[o.Price].push_back(o); // leftover waits
        location[o.id] = {true, o.Price};
    }
}

void addSellOrder(Order o)
{
    // keep matching while a high enough buyer exists
    while (o.qty > 0 && !Buy.empty() && Buy.begin()->first >= o.Price)
    {
        auto &level = Buy.begin()->second; // highest buy price
        Order &resting = level.front();

        int traded = min(o.qty, resting.qty);
        o.qty -= traded;
        resting.qty -= traded;

       
        if (printTrades)
            cout << "Trade: Buyer " << resting.id << " with Seller " << o.id
                 << " | Qty: " << traded << " | Price: " << Buy.begin()->first << endl;

        if (resting.qty == 0)
        {
            location.erase(resting.id);
            level.pop_front();
            if (level.empty())
                Buy.erase(Buy.begin());
        }
    }
    if (o.qty > 0)
    {
        Sell[o.Price].push_back(o);
        location[o.id] = {false, o.Price};
    }
}

// Buy and Sell are different map types, so template
template <typename Book>
void removeFromBook(Book &book, int price, int id)
{
    auto lvl = book.find(price);
    auto &q = lvl->second;
    for (auto it = q.begin(); it != q.end(); ++it)
    {
        if (it->id == id)
        {
            q.erase(it);
            break;
        }
    }
    if (q.empty())
        book.erase(lvl);
}

bool cancelOrder(int id)
{
    auto it = location.find(id);
    if (it == location.end())
        return false; // never existed or already filled
    bool isBuy = it->second.first;
    int price = it->second.second;
    if (isBuy)
        removeFromBook(Buy, price, id);
    else
        removeFromBook(Sell, price, id);
    location.erase(it);
    return true;
}

void printBook()
{
    cout << "--- SELLS ---\n";
    for (auto &s : Sell)
        for (auto &o : s.second)
            cout << "id " << o.id << " @ " << s.first << " qty " << o.qty << endl;
    cout << "--- BUYS ---\n";
    for (auto &b : Buy)
        for (auto &o : b.second)
            cout << "id " << o.id << " @ " << b.first << " qty " << o.qty << endl;
}

void runDemo()
{
    printTrades=1;
    addSellOrder({3, 100, 5, false});
    addSellOrder({4, 105, 5, false});
    addSellOrder({5, 107, 5, false});

    addBuyOrder({1, 105, 8, true});
    addBuyOrder({2, 97, 4, true});   
    addSellOrder({6, 87, 4, false}); 

    cout << (cancelOrder(4) ? "Cancelled" : "Not found") << endl;  // 2 shares of id 4 left
    cout << (cancelOrder(99) ? "Cancelled" : "Not found") << endl; // no such id
    printBook();
}

int main()
{
    runDemo();

    Buy.clear();
    Sell.clear();
    location.clear();
    printTrades = false;

    const int N = 100000; // same N and seed as v1
    mt19937 rng(12345);

    vector<Order> orders; 
    for (int i = 0; i < N; i++)
    {
        int price = 90 + rng() % 21; // 90..110
        int qty = 1 + rng() % 10;    // 1..10
        bool isBuy = rng() % 2;
        orders.push_back({i + 1, price, qty, isBuy});
    }

    auto start = chrono::high_resolution_clock::now();
    for (auto &o : orders)
    {
        if (o.isBuy)
            addBuyOrder(o);
        else
            addSellOrder(o);
    }
    auto end = chrono::high_resolution_clock::now();

    double secs = chrono::duration<double>(end - start).count();
    cout << "Orders: " << N << "\n";
    cout << "Time: " << secs << " s\n";
    cout << "Throughput: " << N / secs << " orders/sec\n";
}