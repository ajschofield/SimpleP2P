import subprocess
import sys

from yaspin import yaspin
from scapy.all import *

def check_permissions():
    return os.geteuid() == 0

def sudo():
    args = [sys.executable] + sys.argv
    os.execlp('sudo', 'sudo', *args)

def run():
    spawn = subprocess.Popen('../build/main', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return spawn

def check_broadcast(packet):
    if "IP" in packet and "UDP" in packet:
        if packet["IP"].dst == "255.255.255.255" and packet["UDP"].dport == 9090:
            return True
        return False

def main():
    if not check_permissions():
        print("This script, as a workaround, requires sudo privileges. Restarting...")
        sudo()
        return

    process = run()

    with yaspin(text="Checking for broadcast message...", color="yellow") as spinner:
        start_time = time.time()
        while time.time() - start_time < 30:
            sniffer = sniff(filter="udp and port 9090", timeout=1, count=1)
            if sniffer and check_broadcast(sniffer[0]):
                spinner.ok("OK")
                break
        else:
            spinner.fail("FAIL")

    process.terminate()
    process.wait()

if __name__ == "__main__":
    main()