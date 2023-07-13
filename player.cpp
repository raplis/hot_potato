#include "connect.h"
#include "potato.h"

//this is the player side of the hot potatp
int main(int argc, char **argv) {
    if(argc != 3) {     //check auguments, they should be 3
        fprintf(stderr, "you should input player <machine_Name> <port_num>\n");
        exit(EXIT_FAILURE);
    }

    const char *machine_Name = argv[1];
    const char *port_num = argv[2];
    // Connect to the ringmaster using the given machine name and port number
    int sock_fd = c_before_con(port_num, machine_Name);
    int self_id = 222;
    int num_player = 223;
    int sig = recv(sock_fd, &self_id, sizeof(self_id), 0);
    // Receive the player's ID from the ringmaster
    if(sig < 0) {
        perror("failure recv 1");
        exit(EXIT_FAILURE);
    }
    // Receive the total number of players from the ringmaster 
    sig = recv(sock_fd, &num_player, sizeof(num_player), 0);
    if(sig < 0) {
        perror("failure recv 2");
        exit(EXIT_FAILURE);
    }
    printf("Connected as player %d out of %d total players\n", self_id, num_player);
    // Create a socket for listening to incoming connections from other players 
    struct sockaddr_in address;
    socklen_t len = sizeof(address);
    int s_fd = s_before_acc("-1");
    getsockname(s_fd, (struct sockaddr*) &address, &len);
    int self_port = ntohs(address.sin_port);
    send(sock_fd, &self_port, sizeof(self_port), 0);
    int r_nei_port;

    recv(sock_fd, &r_nei_port, sizeof(r_nei_port), 0);
    char r_nei_ip_c[20];
    sprintf(r_nei_ip_c, "%d", r_nei_port);

    char r_nei_ip[ADDR_LEN];
    // memset(r_nei_ip, '\0', ADDR_LEN);
    recv(sock_fd, &r_nei_ip, ADDR_LEN, 0);
    // cout << "r_nei_ip" << r_nei_ip << endl;
    // Connect to the right neighbor using the given port number and IP address
    int r_nei_fd = c_before_con(r_nei_ip_c, r_nei_ip);
    // cout << "r_nei_fd" << r_nei_fd << endl;
    string l_ip;
    // Accept a connection from the left neighbor and get its IP address
    int l_nei_fd = s_acc(s_fd, l_ip);
    // cout << "l_nei_fd" << l_nei_fd << endl;

    P potato;
    fd_set fdset;
    vector<int> sel;
    sel.push_back(l_nei_fd);
    sel.push_back(r_nei_fd);
    sel.push_back(sock_fd);
    int maxfd = max(max(l_nei_fd, r_nei_fd), sock_fd);
    //Set the seed for random number generation based on time and player ID
    srand((unsigned int) time(NULL) + self_id);

    while(1) {
        FD_ZERO(&fdset);
        for(size_t i = 0; i < sel.size(); i++) {
            FD_SET(sel[i], &fdset);
        }
        // Wait for any file descriptor in the set to be ready for reading
        select(maxfd + 1, &fdset, NULL, NULL, NULL);
        for(size_t i = 0; i < sel.size(); i++) {
            if(FD_ISSET(sel[i], &fdset)) {
                // cout << "recv signal" << endl;
                // Receive the potato from that file descriptor
                sig = recv(sel[i], &potato, sizeof(potato), 0);
                // cout << potato.count << endl;
                // cout << temp <<endl;
                break;
            }
        }
        if(sig <= 0) {
            break;
        }
        if(potato.hop_num == 0) {
            break;
        }
        else if(potato.hop_num == 1) {
            potato.hop_num--;
            // Add the player's ID to the trace array 
            potato.trace[potato.count] = self_id;
            potato.count++;
            send(sock_fd, &potato, sizeof(potato), 0);
            printf("I'm it\n");
            // printf("I'm it %d\n", self_id);
        }
        else {      // If the hop number is larger than one, the player needs to pass the potato to a neighbor
            potato.hop_num--;
            potato.trace[potato.count] = self_id;
            potato.count++;
            // Generate a random number between 0 and 1 to decide which neighbor to pass to
            int next = rand() % 2;  
            // cout << "next" << next << endl;
            // cout << "l_nei_fd" << l_nei_fd << endl;
            // cout << "r_nei_fd" << r_nei_fd << endl;
            printf("Sending potato to ");
            // printf("Sending potato from %d to ", self_id);
            if(next == 0) {     // Send the potato to left neighbor's file descriptor
                send(l_nei_fd, &potato, sizeof(potato), 0);
                printf("%d\n", (self_id + num_player - 1) % num_player);
            }
            else {  //Send the potato to right neighbor's file descriptor
                send(r_nei_fd, &potato, sizeof(potato), 0);
                printf("%d\n", (self_id + 1) % num_player);
            }
            
        }

    }

    for(size_t i = 0; i < sel.size(); i++) {
        close(sel[i]);
    }
    return EXIT_SUCCESS;
}