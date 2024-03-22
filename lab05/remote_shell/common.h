#pragma once

#include <string>
#include <array>
#include <iostream>
#include <sys/socket.h>

const size_t BUFF_SIZE = 1024;

std::string receive(int sock, bool wait=false) {
    std::string acc;
    {
        std::array<char, BUFF_SIZE+1> buff;
        int bytes_read;
        do {
            bytes_read = recv(sock, buff.data(), BUFF_SIZE, 0);
            buff[bytes_read] = 0;
            if (bytes_read <= 0) break;
            acc += std::string(buff.data());
        } while (bytes_read == BUFF_SIZE || wait);
    }
    return acc;
}
