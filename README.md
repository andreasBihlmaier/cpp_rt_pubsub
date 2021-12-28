cpp_rt_pubsub: C++ real-time publish-subscribe (CRPS)
-----------------------------------------------------
A minimal real-time publish-subscribe mechanism with a central broker implemented in modern C++.

Notes
-----
- While the target platform is Linux, the code should be easy to port to other POSIX platforms by implementing interfaces in os.h and network.h
- To ease detailed analysis, this project intentionally does **not** make use of external libraries, e.g. [Boost.Asio](http://boost.org/libs/asio/)