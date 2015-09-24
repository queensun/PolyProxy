#pragma once
#if (defined(_MSC_VER) && _MSC_VER<1900 && !defined(NOEXCEPT))
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif
#include "except.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <utility>
#include <memory>
#include <chrono>
#include <random>
#include <cstdint>
#include <boost/scoped_ptr.hpp>
#include <boost/scope_exit.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

extern boost::asio::io_service ios;
std::string loadFile(const std::string& filename);
