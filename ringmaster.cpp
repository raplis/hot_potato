#include "potato.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "connect.h"
#include <sys/select.h>

using namespace std;

// This program implements a ringmaster for a hot potato game
// It takes three arguments: port number, number of players and number of hops
int main(int argc, char **argv) {       // Check if the arguments are valid
    if(argc != 4) {
        fprintf(stderr, "the parameter should be: ringmaster <port_num> <num_players> <num_hops>\n");
        exit(EXIT_FAILURE);
    }
    const char *port_num = argv[1];
    const char *num_players = argv[2];
    if(stoi(argv[2]) <= 1) {
        perror("the number of players should greater than 0");
        exit(EXIT_FAILURE);
    }
    const char *num_hops = argv[3];
    int player_num = atoi(num_players);
    int hop_num = atoi(num_hops);
    // int port_n = atoi(port_num);
    printf("Potato Ringmaster\n");
    printf("Players = %d\n", player_num);
    printf("Hops = %d\n", hop_num);

    // Create a server socket and bind it to the given port 
    int ring_sock = s_before_acc(port_num);
    int sig;
    vector<string> ips;
    vector<int> ports;
    vector<int> fds;
    // Loop through each player and accept their connection 
    for(int i = 0; i < player_num; i++) {
        string ip;
        int client_fd = s_acc(ring_sock, ip);

        sig = send(client_fd, &i, sizeof(i), 0);
        if(sig <= 0) {
            perror("failure in send\n");
            exit(EXIT_FAILURE);
        }
        // Send the player's ID and total number of players to them 
        sig = send(client_fd, &player_num, sizeof(player_num), 0);
        if(sig <= 0) {
            perror("failure in send\n");
            exit(EXIT_FAILURE);

        }
        // Receive the player's port number from them 
        int client_port;
        sig = recv(client_fd, &client_port, sizeof(client_port), 0);
        if(sig <= 0) {
            perror("failure in recv\n");
            exit(EXIT_FAILURE);

        }
        ports.emplace_back(client_port);
        fds.emplace_back(client_fd);
        ips.emplace_back(ip);
        cout << "Player " << i << " is ready to play " << endl;
    }

    // Loop through each player and send them their right neighbor's port number and IP address 
    for(int i = 0; i < player_num; i++) {
        int r_nei_id = (i + 1) % player_num;
        int r_nei_port = ports[r_nei_id];
        sig = send(fds[i], &r_nei_port, sizeof(r_nei_port), 0);
        if(sig <= 0) {
            perror("failure in send\n");
            exit(EXIT_FAILURE);

        }
        string ip = ips[r_nei_id];
        char to_send[ADDR_LEN];
        memset(to_send, '\0', ADDR_LEN);
        ip.copy(to_send, ip.length());
        sig = send(fds[i], to_send, ADDR_LEN, 0);

    }

    // Declare a potato structure to store information about hops and trace
    P potato;
    potato.hop_num = hop_num;
    potato.count = 0;
    if(hop_num != 0) {
        // Generate a random number between zero and total number of players-1 as lucky guy who will receive potato first
        srand((unsigned int)time(NULL) + player_num);
        int lucky_guy = rand() % player_num;
        // Send potato structure to lucky guy
        send(fds[lucky_guy], &potato, sizeof(potato), 0);
        if(sig <= 0) {
            perror("failure in sending to lucky guy\n");
            exit(EXIT_FAILURE);

        }
        printf("Ready to start the game, sending potato to player %d\n", lucky_guy);
        fd_set fdset;
        FD_ZERO(&fdset);
        int maxfd = 0;
        for(int i = 0; i < player_num; i++) {   // Add each player's file descriptor to the set
            FD_SET(fds[i], &fdset);
            if(maxfd < fds[i]) {
                maxfd = fds[i];
            }
        }
        // Wait for any player to send back the potato using select function
        select(maxfd + 1, &fdset, NULL, NULL, NULL);
        for(int i = 0; i < player_num; i++) {       // Loop through each player and check who has sent back the potato
            if(FD_ISSET(fds[i], &fdset)) {
                recv(fds[i], &potato, sizeof(potato), 0);
                // cout << temp <<endl;
                // cout << potato.count << endl;
                break;
            }
        }
        
        if(potato.count == 0) {
            fprintf(stderr, "some issue with potato: not zero\n");
            exit(EXIT_FAILURE);
        }
        else {
            P p_zero;
            p_zero.hop_num = 0;
            p_zero.count = 0;
            for(int i = 0; i < player_num; i++) {
                send(fds[i], &p_zero, sizeof(p_zero), 0);
            }
        }
        
        // Print the trace of potato 
        printf("Trace of potato:\n");
        for(int i = 0; i < potato.count - 1; i++) {
            printf("%d,", potato.trace[i]);
        }
        printf("%d\n", potato.trace[potato.count - 1]);
        for(int i = 0; i < player_num; i++) {
            close(fds[i]);
        }
        close(ring_sock);
    }
    return EXIT_SUCCESS;
}