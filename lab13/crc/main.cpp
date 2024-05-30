#include <bits/stdc++.h>

using namespace std;

uint16_t crcu8(uint8_t data, uint16_t crc) {
    uint8_t i = 0, x16 = 0, carry = 0;

    for (i = 0; i < 8; i++) {
        x16 = (uint8_t)((data & 1) ^ ((uint8_t)crc & 1));
        data >>= 1;

        if (x16 == 1) {
            crc ^= 0x4002;
            carry = 1;
        } else {
            carry = 0;
        }

        crc >>= 1;
        if (carry) {
            crc |= 0x8000;
        } else {
            crc &= 0x7fff;
        }
    }
    return crc;
}

uint16_t crcu16(uint16_t newval, uint16_t crc) {
    crc = crcu8((uint8_t) (newval), crc);
    crc = crcu8((uint8_t) ((newval)>>8), crc);
    return crc;
}

uint16_t crcu32(uint32_t newval, uint16_t crc) {
    crc = crcu16((uint16_t) (newval), crc);
    crc = crcu16((uint16_t) ((newval)>>16), crc);
    return crc;
}

uint16_t crcu40(uint64_t newval) {
    uint16_t crc = crcu32((uint32_t) (newval), 0);
    crc = crcu8((uint8_t) ((newval)>>32), crc);
    return crc;
}

struct Package {
    uint64_t data;
    uint64_t crc;
    uint64_t received_data;
};

int main(int argc, char* argv[]) {

    string s;
    cout << "Please enter your message:\n";
    cin >> s;

    mt19937 azino(time(0));
    uniform_int_distribution<int> rnd(0, 80);

    vector<Package> pkgs;
    const size_t pkg_sz = 5;

    for (size_t i = 0; i < s.size(); i += pkg_sz) {
        uint64_t x = 0;
        for (size_t j = i; j < min(i + pkg_sz, s.size()); ++j) {
            x = (x << 8) + s[j];
        }
        auto make_error = [&] (uint64_t data) {
            uint8_t errbit = rnd(azino);
            if (errbit < 40) {
                data ^= (1ull << errbit);
            }
            return data;
        };
        uint64_t crc = crcu40(x);
        pkgs.push_back({x, crc, make_error(x)});
    }

    cout << "Packages:\n";
    for (auto pkg : pkgs) {
        cout << "Next package\n";
        cout << "Sent data: " << pkg.data << "\nCrc data: " << pkg.crc << "\nReceived data: " << pkg.received_data << "\n";
        if (pkg.crc != crcu40(pkg.received_data)) {
            cout << "Package corrupted\n";
        } else {
            cout << "Package correct\n";
        }
        cout << "\n";
    }

    return 0;
}
