#include <iostream>
#include <map>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "common.hpp"
#include "market_manager.hpp"

using namespace boost::placeholders;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::io_service &io_service, MarketManager &market_manager)
            : socket_(io_service), market_manager_(market_manager),
              user_id_(std::numeric_limits<boost::uint64_t>::max()) {}

    boost::asio::ip::tcp::socket &socket() {
        return socket_;
    }

    void Start() {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&Session::HandleRead, shared_from_this(), _1, _2));
    }

private:
    void HandleRead(const boost::system::error_code &error, size_t bytes_transferred) {
        if (!error) {
            data_[bytes_transferred] = '\0';
            auto j = nlohmann::json::parse(data_);
            auto reqType = static_cast<Requests>(j["ReqType"]);
            std::string reply;

            if (reqType == Requests::Registration) {
                reply = HandleRegistration(j);
            } else if (reqType == Requests::ViewBalance) {
                reply = HandleViewBalance(j);
            } else if (reqType == Requests::AddOrder) {
                reply = HandleAddOrder(j);
            } else
                reply = "Error! Unknown request type\n";

            boost::asio::async_write(socket_,
                                     boost::asio::buffer(reply, reply.size()),
                                     boost::bind(&Session::HandleWrite, shared_from_this(), _1));
        }
    }

    void HandleWrite(const boost::system::error_code &error) {
        if (!error)
            Start();
    }

    std::string HandleRegistration(const nlohmann::json &request) {
        if (user_id_ != std::numeric_limits<boost::uint64_t>::max())
            return "You are already registered\n";

        auto user_id = market_manager_.users().size();
        std::string username = request["Username"];
        auto error_code = market_manager_.AddUser(User(user_id, username));
        if (error_code != ErrorCode::OK)
            return "Registration is not successful!\n";

        user_id_ = user_id;
        return "Registration is successful!\n";
    }

    std::string HandleViewBalance(const nlohmann::json &request) {
        if (user_id_ == std::numeric_limits<boost::uint64_t>::max())
            return "You are not registered\n";

        auto user = market_manager_.GetUser(user_id_);
        return "Your balance is: " + std::to_string(user->Balance) + "\n";
    }

    std::string HandleAddOrder(const nlohmann::json &request) {
        boost::uint64_t symbolId = request["SymbolId"];
        OrderSide type = (request["Type"] == "Buy") ? OrderSide::BUY : OrderSide::SELL;
        boost::uint64_t price = request["Price"];
        boost::uint64_t quantity = request["Quantity"];

        auto error_code = market_manager_.AddOrder(Order(market_manager_.GetOrdersCount(), symbolId, user_id_, type, price, 0, quantity));
        if (error_code != ErrorCode::OK)
            return "The order was not created successfully\n";

        return "The order was successfully created\n";
    }

    boost::asio::ip::tcp::socket socket_;
    MarketManager &market_manager_;
    enum {
        max_length = 1024
    };
    char data_[max_length]{};
    boost::uint64_t user_id_;
};

class Server {
public:
    Server(boost::asio::io_service &io_service, MarketManager &market_manager)
            : io_service_(io_service),
              acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT)),
              market_manager_(market_manager) {
        std::cout << "Server started! Listen " << PORT << " port" << std::endl;
        StartAccept();
    }

private:
    void StartAccept() {
        auto new_session = std::make_shared<Session>(io_service_, market_manager_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&Server::HandleAccept, this, new_session, _1));
    }

    void HandleAccept(const std::shared_ptr<Session> &new_session, const boost::system::error_code &error) {
        if (!error) {
            new_session->Start();
            StartAccept();
        }
    }

    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    MarketManager &market_manager_;
};

int main() {
    try {
        boost::asio::io_service io_service;
        MarketManager market_manager;
        const Symbol symbol{0, "USDRUB"};
        market_manager.AddSymbol(symbol);
        market_manager.AddOrderBook(symbol);
        Server server(io_service, market_manager);
        io_service.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}
