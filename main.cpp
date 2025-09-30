#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <ranges>
#include <numeric>
#include <ctime>

class tcp_connection;
class Game;

namespace rv = std::ranges::views;

class RequestUtils {
public:
    enum class Request { MULT, ADD, SUB, DIV, NOT_DEFINED, GAME};

    static Request stringToRequest(const std::string_view& requestStr) {
        if(requestStr.find("*") != std::string_view::npos) {return Request::MULT;}
        if(requestStr.find("-") != std::string_view::npos) {return Request::SUB;}
        if(requestStr.find("/") != std::string_view::npos) {return Request::DIV;}
        if(requestStr.find("+") != std::string_view::npos) {return Request::ADD;}
        if(requestStr.find("GAME") != std::string_view::npos) {return Request::GAME;}

        return Request::NOT_DEFINED;
    }
};

class MathUtils {
public:
    static std::string floatToString(float value) {
        return std::to_string(value);
    }

    static std::string calculate(const std::string& user_request, const RequestUtils::Request& operation) {
        auto result = user_request 
        | rv::filter([](const char n){ return n != '\n'; })
        | rv::split(' ');

        std::vector<float> numbers;

        for (auto token : result) {
            std::string token_str(token.begin(), token.end());

            if (!token_str.empty()) {
                // check if token is numeric and if it's maybe a negative number
                if (std::isdigit(token_str[0]) || (token_str[0] == '-' && token_str.size() > 1 && std::isdigit(token_str[1]))) {
                    try {
                        numbers.push_back(std::stof(token_str));
                        std::cout << "added number " << std::stof(token_str) << " to container" << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error converting '" << token_str << "' to float: " << e.what() << std::endl;
                    }
                }
            }
        }

        switch (operation) {
            case RequestUtils::Request::MULT: {
                float return_value = 1;
                for (auto num : numbers) {
                    return_value *= num;
                }
                return floatToString(return_value);
            }

            case RequestUtils::Request::ADD: {
                float return_value = 0;
                for (auto num : numbers) {
                    return_value += num;
                }
                return floatToString(return_value);
            }

            case RequestUtils::Request::SUB: {
                if (numbers.empty()) return "0";
                float return_value = numbers[0];
                for (auto it = numbers.begin() + 1; it != numbers.end(); ++it) {
                    return_value -= *it;
                }
                return floatToString(return_value);
            }

            case RequestUtils::Request::DIV: {
                if (numbers.size() < 2) return "Invalid operation";
                return numbers[1] == 0 ? "cant divide by 0" : floatToString(numbers[0] / numbers[1]);
            }

            case RequestUtils::Request::NOT_DEFINED:
                return "Entered something not on the list\n";

            default:
                return "Unknown operation";
        }
    }
};


class Game  {

    public:

    //**Definitions are below tcp_connection because we use member functions of tcp_connection class */
    void start(const std::string& message, const std::shared_ptr<tcp_connection>& connection);
    static void ValidateGuess(std::string& guess, const std::shared_ptr<tcp_connection>& connection);

    static bool Get_in_game(){return in_game;}
    static void Set_in_game(bool value){in_game = value;} 

    private:
    size_t user_guess = 0;
    static inline size_t secret_number = 0;
    static inline bool in_game = false;
};

using boost::asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
    using pointer = std::shared_ptr<tcp_connection>;

    static pointer create(boost::asio::io_context& io_context) {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket() {
        return socket_;
    }

    void start() {
        std::cout << std::string_view("someone connected") << std::endl;
        std::string_view message_ = "Welcome!\n";

        boost::asio::async_write(socket_, boost::asio::buffer(message_),
            [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                self->start_reading();
            });
    }

    void send_to_user(const std::string& message) {
        boost::asio::async_write(socket_, boost::asio::buffer(message + "\n"),
            [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                self->start_reading();
            });
    }

    void start_reading() {
        std::cout << "waiting for inputs\n";
        socket_.async_read_some(boost::asio::buffer(buffer_),
            [self = shared_from_this()](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string user_request(self->buffer_.data(), length);
                    if(!Game::Get_in_game()){ //if we dont want to play a game
                    self->handle_request(user_request);}
                    
                    else{self->handle_game_request(user_request);
                        }

                } else {
                    std::cout << "Client disconnected" << std::endl;
                    Game::Set_in_game(false); //removes bug where we are always in game when a new client connects.

                }
            });
    }

    void handle_game_request(std::string& user_guess){
        Game::ValidateGuess(user_guess, shared_from_this());



    }

    void handle_request(std::string& user_request) {
        RequestUtils::Request operation = RequestUtils::stringToRequest(user_request);
        
        switch (operation) {
            case RequestUtils::Request::MULT:
                Game::Set_in_game(false);
                send_to_user(MathUtils::calculate(user_request, RequestUtils::Request::MULT));
                break;
            case RequestUtils::Request::DIV:
                Game::Set_in_game(false);
                send_to_user(MathUtils::calculate(user_request, RequestUtils::Request::DIV));
                break;
            case RequestUtils::Request::ADD:
                Game::Set_in_game(false);
                send_to_user(MathUtils::calculate(user_request, RequestUtils::Request::ADD));
                break;
            case RequestUtils::Request::SUB:
                Game::Set_in_game(false);
                send_to_user(MathUtils::calculate(user_request, RequestUtils::Request::SUB));
                break;
            case RequestUtils::Request::NOT_DEFINED:
                Game::Set_in_game(false);
                send_to_user("you entered something not available. please try again!");
                break;
            case RequestUtils::Request::GAME:
                Game::Set_in_game(true);
                Game game;
                game.start(user_request, shared_from_this());
                break;                                        
        }
    }

    tcp_connection(boost::asio::io_context& io_context) : socket_(io_context) {}

    tcp::socket socket_;
    std::string message_;
    std::array<char, 1024> buffer_;
};

void Game::start(const std::string& message, const std::shared_ptr<tcp_connection>& connection){
    std::cout<<"User chose to play a game!\n";
    connection->send_to_user("Hello you number guessing the game. Please enter a number between 1 and 50");
    std::srand(std::time(0));
    secret_number = std::rand() % 50 + 1;
}

void Game::ValidateGuess(std::string& guess, const std::shared_ptr<tcp_connection>& connection){
    std::cout << "validating guess: " << guess;
    std::cout << "secret number: " << secret_number << "\n";
    //shared pointer to delete the object after each guess
    static int guesses = 1;

    if(guess == std::string("q\n")){
        std::cout << "exiting game\n";
        Game::Set_in_game(false);
        guesses = 1; 
        connection->send_to_user("exiting game");
        connection->start_reading();
    }
    else{ 
        try{
            if(std::stoi(guess) == secret_number){
                connection->send_to_user("correct!");
                Game::Set_in_game(false);
                std::string str_guesses = std::to_string(guesses);
                connection->send_to_user("Required guesses: " + str_guesses);
                connection->send_to_user("exited game");
                guesses = 1;  // Reset for next game
                connection->start_reading();
            }
            else if(std::stoi(guess) > secret_number){
                connection->send_to_user("too high!");
                guesses++;  
            }
            else if(std::stoi(guess) < secret_number){
                connection->send_to_user("too low!");
                guesses++; 
            }
        }
        catch(const std::invalid_argument& e) {
            connection->send_to_user("Invalid input! Please enter a number or 'q' to quit");
        }
    }
}

    
class tcp_server : public std::enable_shared_from_this<tcp_server> {
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234)) {
        start_accept();
    }

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            std::bind(&tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error) {
        if (!error) {
            new_connection->start();
        }
        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main() {
    std::cout << "Program starting..." << std::endl;
    
    try {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        
        std::cout << "Starting to run \n\n\n\n" << std::endl;
        io_context.run();
        std::cout << "finished running" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "exception caught: " << e.what() << std::endl;
    }
    
    std::cout << "exiting" << std::endl;
    return 0;
}
//clang++ -std=c++23 -I/opt/homebrew/opt/boost/include asynch.cpp -o server