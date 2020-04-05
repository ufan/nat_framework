#!/usr/bin/env python3.5
#-*- codig:utf-8 -*-

import socket
import threading
import subprocess
import fcntl    
import os
import select

g_cmd = {
    b"make" : [
        (["svn", "up"], {"cwd" : "/home/hongxu/work/NAT/src/execute_model"}),
        (["make", "clean"], {"cwd" : "/home/hongxu/work/NAT/src"}), 
        (["make", "SDK"], {"cwd" : "/home/hongxu/work/NAT/src"}),
        (["/home/hongxu/work/NAT/release/share/cp2samba.sh"], {})
    ]
}

host = ("127.0.0.1", 9907)

def setunblock(fd):
    flag = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flag | os.O_NONBLOCK)

class CmdProcessor(object):
    def __init__(self, sock):
        object.__init__(self)
        self.sock = sock

    def runOneCmd(self, cmd):
        for command in cmd:
            self.sock.sendall(("execute: %s\n" % " ".join(command[0])).encode("utf8"))
            p = subprocess.Popen(command[0], stdout=subprocess.PIPE, stderr=subprocess.PIPE, **command[1])
            setunblock(p.stdout.fileno())
            setunblock(p.stderr.fileno())
            with select.epoll() as ep:
                ep.register(p.stdout.fileno(), select.EPOLLIN)
                ep.register(p.stderr.fileno(), select.EPOLLIN)
                while p.poll() is None:
                    events = ep.poll(0.5)
                    for fd, ev in events:
                        if ev & select.EPOLLIN:
                            data = b""
                            try:
                                data = (p.stdout if p.stdout.fileno() == fd else p.stderr).read()
                            except Exception as e: print(e)
                            if data: self.sock.sendall(data)
            p.wait()
            if p.returncode != 0:
                self.sock.sendall(b"command exit: %d\n" % p.returncode)
                return
        self.sock.sendall(b"command list execute succ.\n")

    def run(self):
        while True:
            d = self.sock.recv(4096)
            d = d.strip()
            if not d:
                self.sock.close()
                print("client closed")
                break
            print("recv:", d)
            if d not in g_cmd:
                self.sock.sendall(("cannot recognize command:%s\n" % d).encode("utf8"))
            else:
                self.runOneCmd(g_cmd[d])


class CmdServer(object):
    def __init__(self):
        object.__init__(self)

    def init(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.bind(host)
        self.sock.listen(5)

    def listen(self):
        conn, addr = self.sock.accept()
        print('Connected by', addr)
        processor = CmdProcessor(conn)       
        threading.Thread(target=processor.run, daemon=True).start()

    def run(self):
        self.running = True
        while self.running:
            self.listen()


if __name__ == "__main__":
    server = CmdServer()
    server.init()
    t = threading.Thread(target=server.run)
    t.start()
