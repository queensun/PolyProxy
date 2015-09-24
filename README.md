# PolyProxy
This is a project to work around the notorious China firewall (known as the Great Firewall of China, or GFW).

The basic idea is to provide a set of C++ classes and template classes for users to reach the broader internet in a flexible manner. By flexible, I mean a wide range of proxy protocols, and several protocols used in combination and conjunction. Also included in this project are new protocols that make the traffic seem completely random. This is to fight the Deep Package Inspection (DPI). Nowadays, the proliferation of DPI has made most of the proxy protocols based on TLS/SSL unusable.

When the majority of the protocols are finished in future, users can quickly write custom programs according to their own need. Then, a Python binding of the classes and class templates will be designed and included. I'll also consider writing a fully functional program that supports extensive customisation by means of a configure file.

In order to build this project, you need the C++ compilers that support C++11 standard and the libRessl library. Currently this project is for test only. It requires some C++ programming knowledge to do so.
