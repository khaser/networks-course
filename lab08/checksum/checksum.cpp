#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <functional>

#include <cassert>

uint16_t checksum(const std::vector<std::uint16_t> &data) {
    return ~std::accumulate(data.begin(), data.end(), static_cast<uint16_t>(0));
}

bool check_correct(uint16_t chksum, const std::vector<std::uint16_t> &data) {
    return (chksum + static_cast<uint16_t>(~checksum(data)) == static_cast<uint16_t> (0xffff));
}

int main(int argc, char* argv[]) {

    std::vector<std::uint16_t> data(256);
    std::iota(data.begin(), data.end(), 1);

    std::vector<std::function<void(void)>> tests
    {
        [&] {
            assert(check_correct(checksum(data), data));
        },
        [&] {
            assert(!check_correct(checksum(data) + 1, data));
        },
        [&] {
            assert(!check_correct(checksum(data) - 1, data));
        },
        [&] {
            assert(!check_correct(checksum(data), {data.begin() + 10, data.end()}));
        },
        [&] {
            // Known trivial checksum issue
            assert(check_correct(checksum({data.rbegin(), data.rend()}), data));
        }
    };

    for (auto& test : tests) {
        test();
    }
    std::cout << tests.size() << " tests executed successfully\n";
    return 0;
}
