## async-math-game-server
I built this small math + game server as a project to get hands on experience using the boost.asio libary in c++.

# Credit
* the basic structure of this server got taken from the official boost.asio [Tutorial](https://think-async.com/Asio/asio-1.30.2/doc/asio/tutorial/tutdaytime3.html)

# Features
* addition, subtraction, multiplication and division
* Number guessing game where users guess a random number between 1-50

# Prerequisites
* C++23 compatible compiler
* Boost.Asio library

# Building
* Information about compiling boost.asio can be found [here](https://www.boost.org/doc/user-guide/getting-started.html)

# Usage
* Start the server with ./server
* connect to the server with any TCP client on port 1234: nc localhost 1234
* to start the Game simply type GAME

# Examples
10 + 5 + 3    # Returns 18.000000
20 - 5 - 2    # Returns 13.000000
2 * 3 * 4     # Returns 24.000000
15 / 3        # Returns 5.000000

# Example Seesion
* User Responses:
Welcome!
3 * 3
9.000000
2 - 1
1.000000
test    
you entered something not available. please try again!
GAME 
Hello you number guessing the game. Please enter a number between 1 and 50
42
too low!
47
too high!
44
correct!
Required guesses: 3
exited game
* Server Responses:
Program starting...
Starting to run 




someone connected
waiting for inputs
added number 3 to container
added number 3 to container
waiting for inputs
added number 2 to container
added number 1 to container
waiting for inputs
waiting for inputs
User chose to play a game!
waiting for inputs
validating guess: 42
secret number: 44
waiting for inputs
validating guess: 47
secret number: 44
waiting for inputs
validating guess: 44
secret number: 44
waiting for inputs
waiting for inputs
waiting for inputs
waiting for inputs

# What i learned
* proper error handeling can avoid impossible to deciper error messages.
* forward declaration is not enough to tell the compiler the contents of a class -> sometimes i need to move member function definitions below a second class if i want to access methods that are defined inside there.
* * in short: forward declaration only tells the compiler "this class exists" but NOT what's inside it.
* non-const static class variables must be inline if i want to use it with a static member function.
* basic usage of shared_from_this() (returns a shared class pointer) -> somehow makes sure objects stay alive.
* strings can not be used in switch statements but i can mimic strings with enums
* usage of std::string_view in temporary objects to avoid copying (its somewhat dangerous to use with objects that can go out of scope because it can create references to objects that are already deleted. thats bad)
* sending strings over the TCP Server does return the string AND a newline character.
* std::bind is usually replaced by lambdas


