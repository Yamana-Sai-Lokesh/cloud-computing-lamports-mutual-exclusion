// Team Members:
// Sai Lokesh (20CS01041)
// Chochipatla Siddhardh (20CS01008)
// Melam Srihari (20CS01033)

#include "lib/lmpt.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>

/*Lamport.h file*/

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <queue>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

enum Signal {
    REQUEST, //SIGNALS TO REQUEST THE CRITICAL SECTION
    REPLY, //SIGNALS TO REPLY TO A REQUEST
    RELEASE, //SIGNALS TO RELEASE THE CRITICAL SECTION
};

class SyncData {
public:
    int timestamp;
    int senderId;
    Signal msgType;
};

class Lamport {
private:
    int proc_ID; // Unique ID for each process
    int logiClock; // Variable for logical clock
    int listenPort; // Port for listening to incoming messages
    std::mutex clockMutex; // Mutex for protecting logical clock updates
    std::map<int, struct sockadd_in> nodeList; // Map for storing node information
    std::priority_queue<std::pair<int, int>,std::vector<std::pair<int,int>>,std::greater<std::pair<int,int>>> requestQueue; // Queue for storing requests
    std::set<int> replyMap; // Map for storing reply status
public:
    Lamport(int id, int lport);
    ~Lamport();

    // BASE FUNCTIONALITIES

    // ADD SYSTEM TO THE NODE LIST
    void addNode(int id, std::string ip, int port);
    // FIND THE PORT AND SEND THE DATA
    int unicast(Signal sig, int sysId);
    // SEND DATA TO ALL NODES
    int broadcast(Signal sig);
    // RECIEVE DATA FROM THE PORT
    void receive();
    // HANDLE RECIEVED DATA
    void handleData(SyncData data);
    // SEND REQUEST SIGNAL
    void request();
    // MAINTAIN THE REQUEST QUEUE
    void handleQueue();
    // PRINT CONFIG
    void printConfig();
};

/*************************************************************************/

/* Lamport's algorithm*/

Lamport::Lamport(int id, int lport){
    logiClock = 0;
    proc_Id = id;
    listenPort = lport;
}

Lamport::~Lamport(){
,
}


int Lamport::unicast(Signal sig, int sysId){
    // Get sockAdd from nodeList
    struct sockadd_in node = nodeList[sysId];

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Socket creation failed");
        return -1;
    }

    // Connect to the node
    if(connect(sock, (struct sockaddr*)&node, sizeof(node)) < 0){
        perror("Connection failed");
        return -1;
    }

    SyncData data;

    // Lock the clock
    std::unique_lock<std::mutex> lock(clockMutex);

    // Get the clock
    data.timestamp = logicalClock;

    // Set the senderId
    data.senderId = proc_Id;

    // Set the message type
    data.msgType = sig;

    // Unlock the clock
    lock.unlock();

    // Send the data
    if(send(sock, &data, sizeof(data), 0) < 0){
        perror("Send failed");
        return -1;
    }

    // Close the socket
    close(sock);

    return 0;
}

void Lamport::add_Node(int id, std::string ip, int port){
    struct sockadd_in node;
    node.sin_family = AF_INET;
    node.sin_port = htons(port);
    node.sin_addr.s_addr = inet_addr(ip.c_str());
    nodeList[id] = node;
}

int Lamport::broadcast(Signal sig){
    for(auto it = nodeList.begin(); it != nodeList.end(); it++){
        if(it->first != proc_Id){
            unicast(sig, it->first);
        }
    }
    return 0;
}

void Lamport::receive(){
    // Setup server on listenport
    int servsock = socket(AF_INET, SOCK_STREAM, 0);
    if(servsock < 0){
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(listenPort);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(servsock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0){
        perror("Bind failed");
        return;
    }

    SyncData data;

    while(true){
        // Listen for incoming connections
        listen(servsock, 5);

        // Accepting the connection
        int sock = accept(servsock, NULL, NULL);
        if(sock < 0){
            perror("Accept failed");
            return;
        }

        // Receive the data
        if(recv(sock, &data, sizeof(data), 0) < 0){
            perror("Receive failed");
            return;
        }

        // Close the socket
        close(sock);

        // Handle the received data
        handleData(data);
    }
}

void Lamport::handleData(SyncData data){
    // Lock the clock
    std::unique_lock<std::mutex> lock(clockMutex);

    // Update the clock
    logiClock = std::max(logiClock, data.timestamp) + 1;

    // Unlock the clock
    lock.unlock();

    switch(data.msgType){
        case REQUEST:
            std::cout << logiClock << ": Request received from " << data.senderId << std::endl;
            // Add the request to the queue
            requestQueue.push({data.timestamp, data.senderId});

            // If top of requestQueue is not this process, reply
            if(requestQueue.top().second != proc_Id){
                unicast(REPLY, data.senderId);
            }

            break;

        case REPLY:
            // Add reply to the replyMap
            std::cout << logiClock << ": Reply received from " << data.senderId << std::endl;
            replyMap.insert(data.senderId);
            break;

        case RELEASE:
            // Do something
            std::cout << logiClock << ": Release received from " << data.senderId << std::endl;
            if(data.senderId == requestQueue.top().second)
                requestQueue.pop();
            else{
                perror("Invalid release");
                exit(1);
            }
            break;
    }
}

void Lamport::handleQueue(){
    while(true){
        if(!requestQueue.empty()){

            // If the top request is from this process
            if(requestQueue.top().second == proc_Id){

                // Check if all replies are received
                if(replyMap.size() == nodeList.size() - 1){

                    //increment clock
                    std::unique_lock<std::mutex> lock(clockMutex);
                    logiClock++;
                    lock.unlock();

                    std::cout << logiClock << ": Entering Critial Section" << std::endl;
                    // Enter the critical section
                    std::this_thread::sleep_for(std::chrono::seconds(10));

                    // Broadcast release signal
                    broadcast(RELEASE);

                    // Clear the replyMap
                    replyMap.clear();

                    //pop from the pqueue
                    requestQueue.pop();

                    std::cout << logiClock << ": Exiting Critial Section" << std::endl;
                }
            }
        }
    }
}

void Lamport::printConfig(){


    std::cout << "ID: " << proc_Id;
    std::cout << " Port: " << listenPort;
    std::cout << " Clock: " << logiClock << std::endl;

    std::cout << "Node List: " << std::endl;

    for(auto it = nodeList.begin(); it != nodeList.end(); it++){
        std::cout << "ID: " << it->first << " PORT: " << ntohs(it->second.sin_port) << " HOST: " << inet_ntoa(it->second.sin_addr) << std::endl;
    }

    std::cout << "Request Queue: " << std::endl;

    //print the request queue
    std::priority_queue<std::pair<int, int>,std::vector<std::pair<int,int>>,std::greater<std::pair<int,int>>> temp = requestQueue;
    while(!temp.empty()){
        std::cout << "ID: " << temp.top().second << " TIMESTAMP: " << temp.top().first << std::endl;
        temp.pop();
    }

    std::cout << std::endl;

    std::cout << "Reply Map: " << std::endl;

    //Print reply map
    for(auto it = replyMap.begin(); it != replyMap.end(); it++){
        std::cout << "ID: " << *it << std::endl;
    }

    std::cout << std::endl;
}

void Lamport::request(){
    // Increment clock

    std::unique_lock<std::mutex> lock(clockMutex);
    logiClock++;
    lock.unlock();

    broadcast(REQUEST);
    requestQueue.push({logiClock, proc_Id});
}


/****************************************************************************/

/* Main.cpp     */

int main(int argc, char* argv[]) {
    int port = 0;
    int sys_id = 0;
    std::string filename;

    //checking for input in sys_id and port
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " -p <PORT NO> -i <SYS ID> -f <CONFIG FILE> \n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
  std::string arg = argv[i];

  // Check for port argument
  if (arg == "-p") {
    if (i + 1 < argc) {
      try {
        port = std::stoi(argv[++i]); // Convert string to int (using try-catch)
      } catch(const std::invalid_argument& e) {
        std::cerr << "Error: Invalid port number provided.\n";
        return 1;
      }
    } else {
      std::cerr << "Usage: " << argv[0] << " -p <PORT NO>\n";
      return 1;
    }
    // Check for system ID 
    } 
  else if (arg == "-i") {
    if (i + 1 < argc) {
      try {
        sys_id = std::stoi(argv[++i]); // Convert string to int
      } catch(const std::invalid_argument& e) {
        std::cerr << "Error: Invalid sys_ID \n";
        return 1;
      }
    } else {
      std::cerr << "Usage: " << argv[0] << " -i <SYS ID>\n";
      return 1;
    }
    // Checking for filename ( if it exists) 
  } else if (arg == "-f") {
    if (i + 1 < argc) {
      filename = argv[++i];
    } else {
      std::cerr << "Usage: " << argv[0] << " -f <FILENAME>\n";
      return 1;
    }
  // Invalid argument
  } else {
    std::cerr << "Usage: " << argv[0] << " -p <PORT NO> -i <SYS ID> -f <FILENAME>\n";
    return 1;
  }
}
 Lamport* lamport = new Lamport(sys_id, port);

    // Handle config
    std::ifstream file(filename);

    std::vector<std::string> nodeList;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int id, port;
        if (iss >> port >> id) {
            lamport->addNode(id, "127.0.0.1", port);
        }
    }

    // Listener thread used
    std::thread listenerThread(&Lamport::receive, lamport);
    // Queue handler thread initialized
    std::thread queueHandlerThread(&Lamport::handleQueue, lamport);
    // Initialize the input handler thread
    std::thread inputHandlerThread(getInput, lamport);

    // Waiting for listener thread to complete the work
    listenerThread.join();
    // Wait for the queue handler thread to finish
    queueHandlerThread.join();
    // Wait for the input handler thread to finish
    inputHandlerThread.join();

    return 0;
}


void getInput(Lamport* lamport) {
  // Display all available commands
  std::cout << "Commands are:\n";
  std::cout << "- Requesting access (REQUEST)\n";
  std::cout << "- Exiting program (EXIT)\n";
  std::cout << "- Viewing current state (STATUS)\n";

  std::string command;
  while (true) {
    // Prompt for user input
    std::cout << "> ";
    std::cin >> command;

    // Handle user commands
    switch (command) {
      case "REQUEST":
        manager->requestAccess();
        break;
      case "EXIT":
        return;
      case "STATUS":
        manager->printState();
        break;
      default:
        std::cout << "Invalid command not accpeted. Please try again.\n";
    }
  }
}
