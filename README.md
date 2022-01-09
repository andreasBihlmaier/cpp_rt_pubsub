cpp_rt_pubsub: C++ real-time publish-subscribe (CRPS)
-----------------------------------------------------
A minimal real-time publish-subscribe mechanism with a central broker implemented in modern C++.

Design
------
* General
  * A node is a process using CRPS.
  * Topics have priorities. Higher priority topics are always handled before lower priority ones in both broker and nodes.
* Broker
  * Central broker. All data passes through the broker. Publishers send data to the broker. The broker distributes the data to the subscribers.
* Publishers/Subscribers
  * Each process using CRPS must create a `Node` instance that manages the publishers and subscribes in this process.
* Communication
  * Single network connection between node and broker
  * Control: Register Node, add publisher/subscriber, ...
    * Variable length JSON
  * Data: Send/receive data to/from topic
    * Fixed size messages

Limitations / Potential improvements
------------------------------------
* Implementation is rather a proof of concept than production-ready
* Single-threaded
* Single broker (instead of one broker per host to keep communication between nodes on same host local)
* IPv4 only
* No DNS support, only IP addresses
* No support for "latch" / "transient local durability" topics (i.e. deliver latest published message to subscribers that subscribe to topic at later point in time)

Notes
-----
* While the target platform is Linux, the code should be easy to port to other POSIX platforms by implementing interfaces in `os.h` and `network.h`
* To ease detailed analysis, this project intentionally does **not** make use of external libraries, e.g. [Boost.Asio](http://boost.org/libs/asio/)
  * The only exception being [nlohmann/json](https://github.com/nlohmann/json) contained in this repository under [third_party/nlohmann_json](third_party/nlohmann_json)

TODOs
-----
* Real-time on Linux
  * Thread scheduler class and thread priorities
  * Memory management (mlockall, mallopt, pre-load stack, ...)
* All `TODO`s in the code ;)
* Tests
* Documentation
* Implement Node (and hence Publisher and Subscriber) unregister
* Implement API to query Broker, e.g. about nodes, topics, message types, publishers, subscribers, etc.
* Sort source file content (e.g. method order).
* General cleanup
* Implement `Network::Protocol::TCP`
* Handle wrap around of `BpHeader::counter`
* Make implementation robust against corrupted / not well-formed messages, esp. `BpType::Control`.
* Distinct `error_code`s in Control JSON
* Accept dependency on [Boost.Program_options](https://www.boost.org/libs/program_options) and cleanup `*_main.cpp`(?)
* Isolate `BpType::Data` processing from `BpType::Control` processing in broker.
* Increment and monitor `BpHeader::counter` also in direction broker->node (not just node->broker); Counter should be unique per node
* Add node liveliness monitoring. Either continuous (heartbeat from node to broker) or sporadic (broker polling node on demand, e.g. when node with same name tries to register)
* Add a Broker->Node API?
* Add a Node->(Broker->)Node API?
* No-subscriber-optimization (i.e. don't publish data to broker that has no subscribers (unless topic is "latching"))
* Add an example pub/sub that shows how to use CRPS with a proper (fixed sized) message serialization library (e.g. ROS2, FlatBuffers or Protocol Buffers) on top. Include a Publisher/Subscriber class, templated on the message type, that use `Publisher`/`Subscriber` internally, but hide all the raw buffer operations from the user.
