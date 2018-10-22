#include <memory.h>

#include <bmcpp/array.hpp>
#include <bmcpp/list.hpp>
#include <bmcpp/hashmap.hpp>
#include <bmcpp/string.hpp>

using namespace std;
using namespace BmCpp;

struct A : BaseAllocation {
    virtual ~A()    = 0;
};

A::~A() {
    fprintf(stderr, "~A\n");
}

struct B : A {
    B(const int* a, size_t len) {
        for( size_t i = 0; i < len; ++i ) {
            this->a.pushBack(a[i]);
        }
    }

    ~B() override {
        fprintf(stderr, "%d\n", a[0]);
    }

    Array<int>  a;
};

int main()
{
    fprintf(stderr, "testing\n");
    int a[] = {1, 2, 3};
    auto b  = new B(a, 3);
    auto l = List<int>();
    auto iter = l.insert(l.begin(), 10);
    fprintf(stderr, "list length: %lu\n", l.size());
    l.erase(iter);

    HashMap<uint32_t, uint32_t> hm;
    hm.set(10, 100);
    hm.set(11, 110);
    fprintf(stderr, "hash size: %d\n", hm.count());

    delete b;
    return 0;
}
