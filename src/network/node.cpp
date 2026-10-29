#include <atomic>
#include <chrono>
#include "network/network.hpp"
#include "network/tcp_server.hpp"
#include "network/tcp_transport.hpp"
#include "network/message.hpp"

#include "command/command.hpp"
#include "kv/store.hpp"
#include "storage/wal.hpp"

#include "raft/raft.hpp"
#include "raft/raft_message.hpp"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: node <id>\n";
        return 1;
    }

    NodeId my_id;

    try
    {
        my_id = static_cast<NodeId>(std::stoul(argv[1]));
    }
    catch (const std::exception &)
    {
        std::cerr << "Invalid node ID. Use 1, 2, or 3.\n";
        return 1;
    }

    std::unordered_map<NodeId, NodeInfo> nodes{
        {1, {1, "127.0.0.1", 5001}},
        {2, {2, "127.0.0.1", 5002}},
        {3, {3, "127.0.0.1", 5003}}};

    if (nodes.find(my_id) == nodes.end())
    {
        std::cerr << "Unknown node ID. Use 1, 2, or 3.\n";
        return 1;
    }

    try
    {
        initialize_network();

        const auto &me = nodes.at(my_id);

        TcpServer server(me.port);
        TcpTransport transport(nodes);

        // Each node has its own WAL file.
        WAL wal("node_" + std::to_string(my_id) + ".wal");
        KVStore store(wal);

        // Restore the previously applied key-value state.
        store.recover();

        Raft raft(my_id, nodes, transport, store);

        std::cout << "Node " << my_id
                  << " started as a follower; waiting for election\n";

        std::atomic<bool> running{true};

        std::cout << "Node " << my_id
                  << " listening on port "
                  << me.port << '\n';

        std::thread receiver([&]()
                             {
            while (true) {
                try {
                    auto connection = server.accept();
                    Message message = connection.receive();
                    if (message.type == MessageType::RequestVote) {
    RequestVote request =
        deserialize_request_vote(message.payload);

    RequestVoteResponse response =
        raft.handle_request_vote(request);

    Message reply{
        MessageType::RequestVoteResponse,
        my_id,
        message.sender,
        serialize_request_vote_response(response)
    };

    transport.send(message.sender, reply);

    std::cout << "\n[Node " << my_id
              << "] RequestVote from Node " << message.sender
              << ", term=" << request.term
              << ", granted=" << response.vote_granted
              << "\n> " << std::flush;
}
else if (message.type == MessageType::RequestVoteResponse) {
    RequestVoteResponse response =
        deserialize_request_vote_response(message.payload);

    raft.handle_request_vote_response(
        message.sender,
        response
    );

    std::cout << "\n[Node " << my_id
              << "] RequestVoteResponse from Node "
              << message.sender
              << ", term=" << response.term
              << ", granted=" << response.vote_granted
              << ", current term=" << raft.current_term()
              << "\n> " << std::flush;
}
                    else if (message.type == MessageType::AppendEntries) {
                        AppendEntries request =
                            deserialize_append_entries(message.payload);

                        AppendEntriesResponse response =
                            raft.handle_append_entries(request);

                        Message reply{
                            MessageType::AppendEntriesResponse,
                            my_id,
                            message.sender,
                            serialize_append_entries_response(response)
                        };

                        transport.send(message.sender, reply);

                        // std::cout << "\n[Node " << my_id
                        //           << "] processed AppendEntries from Node "
                        //           << message.sender
                        //           << ", term=" << request.term
                        //           << ", success=" << response.success
                        //           << ", matchIndex=" << response.match_index
                        //           << "\n> " << std::flush;
                    }
                    else if (
                        message.type == MessageType::AppendEntriesResponse
                    ) {
                        AppendEntriesResponse response =
                            deserialize_append_entries_response(
                                message.payload
                            );

                        // std::cout << "\n[Node " << my_id
                        //           << "] received AppendEntriesResponse from Node "
                        //           << message.sender
                        //           << ", term=" << response.term
                        //           << ", success=" << response.success
                        //           << ", matchIndex=" << response.match_index
                        //           << "\n> " << std::flush;

                        raft.handle_append_entries_response(
                            message.sender,
                            response
                        );
                    }
                    else {
                        std::cout << "\n[Node " << my_id
                                  << "] received message from Node "
                                  << message.sender
                                  << ": " << message.payload
                                  << "\n> " << std::flush;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "\nReceiver error: "
                              << e.what() << '\n';
                }
            } });

        receiver.detach();
        std::thread ticker([&]() {
    while (running.load()) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(25)
        );

        if (running.load()) {
            try {
                raft.tick();
            }
            catch (const std::exception& e) {
                std::cerr << "\nRaft timer error: "
                          << e.what() << '\n';
            }
        }
    }
});
        std::cout << "\nCommands:\n";
        std::cout << "  set <key> <value>\n";
        std::cout << "  get <key>\n";
        std::cout << "  delete <key>\n";
        std::cout << "  heartbeat\n";
        std::cout << "  quit\n";
        std::cout << "> " << std::flush;

        std::string command;

        while (std::getline(std::cin, command))
        {
            if (command == "quit")
            {
                break;
            }
            else if (command == "heartbeat")
            {
                try
                {
                    raft.send_heartbeats();
                    std::cout << "Heartbeat/replication broadcast requested\n";
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Heartbeat error: "
                              << e.what() << '\n';
                }
            }
            else if (command.rfind("set ", 0) == 0)
            {
                std::istringstream input(command.substr(4));

                std::string key;
                std::string value;

                if (!(input >> key >> value))
                {
                    std::cout << "Usage: set <key> <value>\n";
                }
                else if (raft.propose(Command::set(key, value)))
                {
                    std::cout
                        << "SET accepted into the Raft log; "
                        << "waiting for majority commitment\n";
                }
                else
                {
                    std::cout
                        << "SET rejected: this node is not the leader\n";
                }
            }
            else if (command.rfind("get ", 0) == 0)
            {
                std::string key = command.substr(4);

                if (key.empty() || key.find_first_of(" \t") != std::string::npos)
                {
                    std::cout << "Usage: get <key>\n";
                }
                else
                {
                    auto value = raft.get(key);

                    if (value.has_value())
                    {
                        std::cout << *value << '\n';
                    }
                    else
                    {
                        std::cout << "(not found)\n";
                    }
                }
            }
            else if (command.rfind("delete ", 0) == 0)
            {
                std::string key = command.substr(7);

                if (key.empty() || key.find_first_of(" \t") != std::string::npos)
                {
                    std::cout << "Usage: delete <key>\n";
                }
                else if (raft.propose(Command::remove(key)))
                {
                    std::cout
                        << "DELETE accepted into the Raft log; "
                        << "waiting for majority commitment\n";
                }
                else
                {
                    std::cout
                        << "DELETE rejected: this node is not the leader\n";
                }
            }
            else
            {
                std::cout << "Unknown command\n";
            }

            std::cout << "> " << std::flush;
        }
        running.store(false);

if (ticker.joinable()) {
    ticker.join();
}

shutdown_network();
return 0;
        
    }
    catch (const std::exception &e)
    {
        std::cerr << "Node error: " << e.what() << '\n';

        shutdown_network();
        return 1;
    }
}
