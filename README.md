# Connect Four Game

A C++ implementation of the classic Connect Four game with a client-server architecture and multiple bot players.

## Features

- WebSocket-based client-server architecture
- Multiple bot implementations:
  - Random Luka Bot: Makes completely random moves
  - Random Janez Bot: Prioritizes center column with fallback to random moves
- SQLite database for player statistics and ELO ratings
- Interactive gameplay through console interface

## Prerequisites

- C++20 compatible compiler
- WebSocket++ library (included in the repository)
- JsonCpp library (included in the repository)
- SQLite3 (included in the repository)
- CMake (version 3.10 or higher)

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/connect-four.git
cd connect-four
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Generate build files with CMake:
```bash
cmake ..
```

Additionally, you can specify BOOST_ROOT to point to your Boost installation directory.
```bash
cmake -DBOOST_ROOT=/path/to/boost ..
```

4. Build the project:
```bash
cmake --build .
```

## Running the Game

1. Start the server:
```bash
./server
```

2. In separate terminal windows, run clients or bots:
```bash
./client <server_uri> (e.g., ws://localhost:9002)  # For human player
./random_luka <server_uri> (e.g., ws://localhost:9002)  # For Random Luka bot
./random_janez <server_uri> (e.g., ws://localhost:9002)  # For Random Janez bot
```

## Project Structure

- `server.cpp/h`: Server implementation
- `client.cpp`: Human player client implementation
- `bot.cpp/h`: Base bot class implementation
- `random_luka.cpp`: Random move bot implementation
- `random_janez.cpp/h`: Center-prioritizing bot implementation
- `ConnectFourGame.cpp/h`: Game logic implementation
- `DatabaseManager.cpp/h`: SQLite database management

## Documentation

The project uses Doxygen for documentation. To generate the documentation:

1. Install Doxygen
2. Run:
```bash
doxygen Doxyfile
```
3. Open `docs/html/index.html` in your web browser

## License

MIT License

Copyright (c) 2024 Valentin Kragelj

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
