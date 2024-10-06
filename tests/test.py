import subprocess
from yaspin import yaspin
import time
from scapy.all import *

def run():
    spawn = subprocess.Popen('../build/main', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return spawn

def check_broadcast(packet):
    if "IP" in packet and "UDP" in packet:
        if packet["IP"].dst == "255.255.255.255" and packet["UDP"].dport == 9090:
            return True
        return False

process = run()

with yaspin(text="Checking for broadcast message...", color="yellow") as spinner:
    start_time = time.time()
    while time.time() - start_time < 30:
        sniff = sniff(filter="udp and port 9090", timeout=1, count=1)
        if sniff and check_broadcast(sniff[0]):
            spinner.ok("OK")
            break
    else:
        spinner.fail("FAIL")

process.terminate()
process.wait()
