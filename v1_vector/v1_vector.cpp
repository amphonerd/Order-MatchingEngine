// Order Matching Engine - v1 (baseline: sorted vectors)
// Slow by design: re-sorts on every insert and shifts elements on erase.
// Build: g++ -O2 -std=c++14 v1_vector.cpp -o v1 && ./v1

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>

using namespace std;

struct Order
{
    int id;
    int Price;
    int qty;
    bool isBuy;
};

bool printTrades = true; // turn off for the benchmark,printing is slow

vector<Order> Buy;  // highest price first
vector<Order> Sell; // lowest price first

void addBuyOrder(Order o)
{
    // keep matching while buy has quantity and a cheap enough seller exists
    while (o.qty > 0 && !Sell.empty() && Sell[0].Price <= o.Price)
    {
        int traded = min(o.qty, Sell[0].qty);
        o.qty -= traded;
        Sell[0].qty -= traded;

        if (printTrades)
        {
            cout << "Trade: Buyer " << o.id << " with Seller " << Sell[0].id
                 << " | Qty: " << traded << " | Price: " << Sell[0].Price << endl;
        }

        if (Sell[0].qty == 0)
        {
            Sell.erase(Sell.begin()); // seller fully used up
        }
    }

    // leftover quantity
    if (o.qty > 0)
    {
        Buy.push_back(o);
        // stable so same price keeps arrival order
        stable_sort(Buy.begin(), Buy.end(),
                    [](const Order &a, const Order &b)
                    { return a.Price > b.Price; });
    }
}
void addSellOrder(Order o)
{

    // keep matching while the sell has quantity and a high enough buyer exists
    while (o.qty > 0 && !Buy.empty() && Buy[0].Price >= o.Price)
    {
        int traded = min(o.qty, Buy[0].qty);
        o.qty -= traded;
        Buy[0].qty -= traded;

        if (printTrades)
        {
            cout << "Trade: Buyer " << Buy[0].id << " with Seller " << o.id
                 << " | Qty: " << traded << " | Price: " << Buy[0].Price << endl;
        }

        if (Buy[0].qty == 0)
        {
            Buy.erase(Buy.begin()); //  fully used up
        }
    }

    // leftover quantity
    if (o.qty > 0)
    {
        Sell.push_back(o); 
        // stable so same price keeps arrival order
        stable_sort(Sell.begin(), Sell.end(), 
                    [](const Order &a, const Order &b)
                    { return a.Price < b.Price; });
    }
}

void printBook()
{
    cout << "--- SELLS ---\n";
    for (auto &s : Sell)
        cout << "id " << s.id << " @ " << s.Price << " qty " << s.qty << endl;
    cout << "--- BUYS ---\n";
    for (auto &b : Buy)
        cout << "id " << b.id << " @ " << b.Price << " qty " << b.qty << endl;
}
bool cancelOrder(int id)
{

    for (size_t i = 0; i < Buy.size(); i++)
    {
        if (Buy[i].id == id)
        {
            Buy.erase(Buy.begin() + (i));
            return true;
        }
    }
    for (size_t i = 0; i < Sell.size(); i++)
    {
        if (Sell[i].id == id)
        {
            Sell.erase(Sell.begin() + (i));
            return true;
        }
    }
    return false;
}

void runDemo()
{
    addSellOrder({3, 100, 5, false});
    addSellOrder({4, 105, 5, false});
    addSellOrder({5, 107, 5, false});

    addBuyOrder({1, 105, 8, true});
    addBuyOrder({2, 97, 4, true});
    addSellOrder({6, 87, 4, false});

    cout << (cancelOrder(4) ? "Cancelled" : "Not found") << endl;  // leftover of id 4
    cout << (cancelOrder(99) ? "Cancelled" : "Not found") << endl; // never existed
    printBook();
}

int main()
{
    runDemo(); 

    Buy.clear();
    Sell.clear(); // start the benchmark with an empty book
    printTrades = false;

    const int N = 50000; 
    mt19937 rng(12345);  // fixed seed = same orders every run

    // generate all orders
    vector<Order> orders;
    for (int i = 0; i < N; i++)
    {
        int price = 90 + rng() % 21; // 90..110
        int qty = 1 + rng() % 10;    // 1..10
        bool isBuy = rng() % 2;
        orders.push_back({i + 1, price, qty, isBuy});
    }

    cout<<"Benchmarking...Pls wait"<<'\n';

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
    cout << "Orders: " << N << '\n';
    cout << "Time: " << secs << " s\n";
    cout << "Throughput: " << N / secs << " orders/sec\n";
}
