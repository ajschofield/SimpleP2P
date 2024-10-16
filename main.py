import sys
import os
import socket
import multiprocessing
import signal
import time
from getkey import getkey, keys
from cffi import FFI

INIT_STRING = """
------------------------
Name: SimpleP2P
Author: Alex Schofield
------------------------
"""

ffi = FFI()

ffi.cdef("""
struct sockaddr_in {
    short            sin_family;
    unsigned short   sin_port;
    unsigned int     sin_addr;
    char             sin_zero[8];
};

struct peer_info {
    struct sockaddr_in addr;
    int discovery_socket;
};

struct peer_info discover_peer();
int establish_connection(struct sockaddr_in peer_addr, int discovery_socket);
void close(int fd);
""")

if getattr(sys, "frozen", False):
    application_path = sys._MEIPASS
else:
    application_path = os.path.dirname(__file__)

lib_path = os.path.join(application_path, "build", "lib_simplep2p.so")

C = ffi.dlopen(lib_path)


def discover_and_connect():
    peer = C.discover_peer()

    peer_ip_int = socket.ntohl(peer.addr.sin_addr)
    peer_ip_bytes = peer_ip_int.to_bytes(4, "big")
    peer_ip = socket.inet_ntoa(peer_ip_bytes)
    peer_port = socket.ntohs(peer.addr.sin_port)

    print(f"Discovered peer at {peer_ip}:{peer_port}")

    connection_fd = C.establish_connection(peer.addr, peer.discovery_socket)
    if connection_fd < 0:
        pass
    else:
        print("Keeping connection alive for 10 seconds...")
        time.sleep(1)
        C.close(connection_fd)


if __name__ == "__main__":
    print(INIT_STRING)

    process = multiprocessing.Process(target=discover_and_connect)
    process.start()

    try:
        print("Press 'ESC' to quit.")
        while process.is_alive():
            key = getkey()
            if key is not None:
                if key == keys.ESC:
                    print("Quitting...")
                    os.kill(process.pid, signal.SIGKILL)
                    break
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("Quitting...")
        os.kill(process.pid, signal.SIGKILL)

    process.join()
    print("Goodbye!")
