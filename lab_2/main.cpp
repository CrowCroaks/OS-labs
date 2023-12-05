#include <csignal>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <map>

using namespace std;

const int MAX_CLIENTS = 5;
const int BUFFER_SIZE = 512;

struct client {
    public:
        int descriptor;
        sockaddr_in address;
};

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r){
    wasSigHup = 1;
}

void sigHandRegistration(){
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);
}

void sigBlocking(sigset_t origMask){
    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);
}

int createServ(int port){
    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd == -1){
        cerr << "Socket creation failed" << endl;
        exit(-1);
    }

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
        cerr << "Socket bind failed" << endl;
        exit(-1);
    }

    if (listen(servfd, MAX_CLIENTS) != 0){
        cerr << "Listen failed" << endl;
        exit(-1);
    }   

    return servfd;
}

int main(){

    int servfd = createServ(5005);
    cout << "Starting..." << endl;

    client clients[MAX_CLIENTS] = {};
    int active_clients = 0;
    string buffer;

    sigset_t origSigMask;
    sigHandRegistration();
    sigBlocking(origSigMask);

    while (true) {
        if (wasSigHup) {
            wasSigHup = 0;
            cerr << "Current connections: " << active_clients << endl;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(servfd, &fds);
        int maxFd = servfd;

        for (int i = 0; i < active_clients; i++) {
            FD_SET(clients[i].descriptor, &fds);
            if (clients[i].descriptor > maxFd)
                maxFd = clients[i].descriptor;
        }

        if (pselect(maxFd + 1, &fds, nullptr, nullptr, nullptr, &origSigMask) < 0) {
            if (errno == EINTR)
                continue;
            cerr << "pselect failed" << endl;
            return 0;
        }

        if (FD_ISSET(servfd, &fds) && active_clients < MAX_CLIENTS) {
            client* client = &clients[active_clients];
            socklen_t addrsize = sizeof(client->address);
            int descriptor = accept(servfd, (struct sockaddr*)&client->address, &addrsize);
            if (descriptor >= 0) {
                client->descriptor = descriptor;
                cerr << "Connected" << endl;
                active_clients++;
            } else {
                cerr << "Accept error" << endl;
            }
        }

        for (int i = 0; i < active_clients; i++) {
            client* client = &clients[i];
            if (FD_ISSET(client->descriptor, &fds)) {
                buffer.clear();
                int readlen = read(client->descriptor, &buffer[0], BUFFER_SIZE - 1);
                if (readlen > 0) {
                    buffer[readlen - 1] = 0;
                    cerr << "Message: " << buffer.c_str() << endl;
                } else {
                    close(client->descriptor);
                    cerr << "Connection closed" << endl;
                    clients[i] = clients[active_clients - 1];
                    active_clients--;
                    i--;
                }
            }
        }
    }
}
