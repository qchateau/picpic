#pragma once

#include <filesystem>
#include <boost/coroutine2/all.hpp>

namespace picpack {

typedef boost::coroutines2::coroutine<std::filesystem::path> coro_t;

} // picpack
