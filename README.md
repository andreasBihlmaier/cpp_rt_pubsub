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

Notes
-----
* While the target platform is Linux, the code should be easy to port to other POSIX platforms by implementing interfaces in `os.h` and `network.h`
* To ease detailed analysis, this project intentionally does **not** make use of external libraries, e.g. [Boost.Asio](http://boost.org/libs/asio/)

TODOs
-----
* Tests
* Documentation
* Implement `Network::Protocol::TCP`
* Handle wrap around of `BpHeader::counter`