oimport socket
import time
SERVER = "127.0.0.1"
PORT = 6666

def connect_to_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER, PORT))
    return s

def send(s, cmd):
    s.sendall((cmd + "\r\n").encode())
    print(">>>", cmd)

def auth(s, nick):
    send(s, "PASS pass")
    time.sleep(0.05)
    send(s, f"NICK {nick}")
    time.sleep(0.05)
    send(s, "USER tester 0 * :Simple IRC client")
    time.sleep(0.05)

def main():
    print("####### tester ########")
    s1 = connect_to_server()
    auth(s1, "tester")

    data = s1.recv(4096)
    print(data.decode(errors='ignore'))
    
    print("####### rune #######")
    s2 = connect_to_server()
    auth(s2, "rune")

    data2 = s2.recv(4096)
    print(data2.decode(errors='ignore'))
    send(s2, "JOIN #rune")
    data2 = s2.recv(4096)
    print(data2.decode(errors='ignore'))
    
    send(s2, "MODE #rune +k password")
    data2 = s2.recv(4096)
    print(data2.decode(errors='ignore'))

    s3 = connect_to_server()
    auth(s3, "ggg")
    send(s3, "JOIN #rune")
    data2 = s2.recv(4096)
    print(data2.decode(errors='ignore'))
    
    s1.close()
    s2.close()

if __name__ == "__main__":
    main()

