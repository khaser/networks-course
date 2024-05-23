#!/usr/bin/env python3

import socket
from struct import pack, unpack
from time import time
import sys

import argparse

parser =  argparse.ArgumentParser('traceroute')
parser.add_argument('-d', '--hostname', type=str, required=True)
parser.add_argument('-c', '--count', type=int, default=3)
parser.add_argument('-t', '--timeout', type=int, default=1)
parser.add_argument('-m', '--max-hops', type=int, default=64)


ICMP_ECHO = 8
ICMP_FORMAT = '!BBHHH'

def checksum(data):
    if len(data) % 2:
        data += b'\x00'
    res = sum(unpack('!H', data[i:i+2])[0] for i in range(0, len(data), 2))
    while res >> 16:
        res = (res >> 16) + (0xFFFF & res)
    return 0xFFFF & ~res


if __name__ == "__main__":
    args = parser.parse_args()

    try:
        dest_addr = socket.gethostbyname(args.hostname)
    except socket.gaierror:
        print("Fail to resolve hostname: ", args.hostname)
        exit(1)

    for ttl in range(1, args.max_hops + 1):
        ttl_flag = False
        with socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.getprotobyname("icmp")) as sock:
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_TTL, ttl)
            sock.settimeout(args.timeout)

            rtts = []
            addr = None
            for i in range(args.count):
                header = pack(ICMP_FORMAT, ICMP_ECHO, 0, 0, 1, ttl)
                data = pack('d', time())
                header = pack(ICMP_FORMAT, ICMP_ECHO, 0, checksum(header + data), 1, ttl)

                send_time = time()
                sock.sendto(header + data, (dest_addr, 0))
                try:
                    _, addr = sock.recvfrom(1024)
                    rtt = round((time() - send_time) * 1000, 2)
                    rtts.append(rtt)
                except :
                    rtts.append("***")

            if addr != None:
                try:
                    hostname, _, addr = socket.gethostbyaddr(addr[0])
                except:
                    hostname = "unrecognized"
            print(f"{ttl}, {addr[0]} ({hostname}):", *rtts)
            if (addr[0] == dest_addr):
                exit(0)
