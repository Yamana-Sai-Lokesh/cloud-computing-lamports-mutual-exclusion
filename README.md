# Lamport's Mutual Exclusion Algorithm
This project implements Lamport's Mutual Exclusion Algorithm
## Project Structure

```
.
├── Makefile
├── README.md
└── main.cpp

```

- `Makefile`: Contains rules for building the project.
- `README.md`: This file, providing an overview of the project.
- `main.cpp`: Entry point of the application, handles command-line arguments and initializes the algorithm.\
   Implementation of the Lamport's Mutual Exclusion Algorithm. Header file containing the class declaration and necessary data structures.

## Building the Project

To build the project, navigate to the project directory and run the following command:

```
make
```

This will compile the source files and generate the executable `main`.

## Running the Project

After building the project, you can run the executable with the following command:

```
./main.cpp -p <PORT_NO> -i <SYSTEM_ID> -f <CONFIG_FILE>
```

```
8080 1
8081 2

```

and so on

The program will start listening for incoming messages on the specified port and will wait for user input.
During execution, the program will prompt you to enter additional commands:

- `REQUEST`: Send a request for accessing the critical section.
- `STATUS`: Add a new node to the distributed system.
- `EXIT`: Terminate the program.

## Lamport Class

The `Lamport` class is a C++ implementation of Lamport's distributed mutual exclusion algorithm. It provides a way to ensure that only one node is in its critical section at a time in a distributed system.

### Class Members

- `int logiClock`: An atomic variable representing the logical clock of the node.
- `int listenPort`: The port number on which the node listens for incoming messages.
- `std::mutex clockMutex`: A mutex used to protect updates to the logical clock.
- `std::map<int, struct sockadd_in> nodeList`: A map storing information about other nodes in the system. The key is the node ID and the value is the node's address.
- `std::priority_queue<std::pair<int, int>> requestQueue`: A priority queue storing requests from other nodes. Each request is a pair of integers representing the timestamp and the node ID.
- `std::set<int> replyMap`: A set storing the reply status of other nodes.

### Constructor and Destructor

- `Lamport(int id, int lport)`: Constructor that initializes a new `Lamport` object with a given node ID and listening port.
- `~Lamport()`: Destructor that cleans up the `Lamport` object when it is no longer needed.

### Member Functions

- `void addNode(int id, std::string ip, int port)`: Adds a new node to the `nodeList` with the given ID, IP address, and port number.
- `int unicast(Signal sig, int sysId)`: Sends a signal to the node with the given system ID. The signal is sent via unicast.

This class provides the basic functionalities needed to implement Lamport's distributed mutual exclusion algorithm in a distributed system.

## Team Members

- Yamana Sai Lokesh (20CS01041)
- Chochipatla Siddhardh (20CS01008)
- Melam Srihari (20CS01033)

## Algorithm Overview

Here are the key points of Lamport's Mutual Exclusion Algorithm in our Algorithm:

1. Processes maintain logical clocks incremented with events.
2. Process seeking critical section access broadcasts timestamped requests.
3. Processes update clocks, reply to requests, allowing one process at a time in the critical section.

 
 
