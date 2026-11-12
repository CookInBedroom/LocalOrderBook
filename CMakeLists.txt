# Project required CMake version
cmake_minimum_required(VERSION 4.3)

# Project name
project(TradingSystemCpp20)

# Set C++20 build standard
set(CMAKE_CXX_STANDARD 20)

# Makes C++20 standard mandatory
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions
# Keeps the code closer to standard C++20
set(CMAKE_CXX_EXTENSIONS OFF)

# Expose JThread and stop_source as they may be hidden when using Apple Clang 17 on macOS
add_compile_options(-fexperimental-library)

# Include the MarketData library
add_subdirectory(src/MarketData)

# Create the single executable target for the TradingSystemCpp20 project
add_executable(TradingSystemCpp20 main.cpp)

# Links the MarketData library to the executable
target_link_libraries(TradingSystemCpp20 PRIVATE MarketData)
