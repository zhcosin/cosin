
#include "include.h"
#include "utils/utils.h"
#include "log/logger.h"
#include "tcp/tcpserver.h"

namespace apdu {

tcp_server_session::tcp_server_session(boost::asio::io_service &io_service, 
									   const tcp_server_session_read_cb& read_cb, 
									   const tcp_server_session_close_cb session_close_cb)
: socket_(io_service)
, logcollector(std::string("tcp-server-connect-") + boost::lexical_cast<std::string>(get_id()), 0x0)
, read_cb_(read_cb)
, close_cb_(session_close_cb)
{
	memset(recv_buf, 0, sizeof(recv_buf));
}

tcp_server_session::~tcp_server_session()
{
}

void tcp_server_session::start() 
{
	static boost::asio::ip::tcp::no_delay option(true);
	socket_.set_option(option);

	socket_.async_read_some(
		boost::asio::buffer(recv_buf, sizeof(recv_buf)),
		boost::bind(&tcp_server_session::handle_read,
		shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket &tcp_server_session::socket() 
{
	return socket_;
}

unsigned int tcp_server_session::write(const unsigned char *message, unsigned int size)
{
	if (message == NULL || size == 0)
		throw std::invalid_argument("the message to send is a empty pointer or the length is zero.");

	if (socket_.is_open())
	{
		boost::asio::ip::tcp::endpoint remote_ept(socket_.remote_endpoint());

		std::stringstream ss;
		ss << "send " << size << " bytes data to: [" << remote_ept.address() << "]:" << remote_ept.port() << "\n";
		utils::out_buffer_in_hex(ss, (unsigned char *)message, size);
		logger::instance().log(logcollector, log_trace, ss.str());

		return socket_.write_some(boost::asio::buffer(message, size));
	}
	else
		throw(std::runtime_error("can't send message because the tcp client has not been open."));
}

void tcp_server_session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) 
{
	if (error) 
	{
		std::stringstream ss;
		ss << "read with error: " << error << " bytes_transferred: " << bytes_transferred << ".";
		logger::instance().log(logcollector, log_error, ss.str());

		if (close_cb_)
			close_cb_(get_id());
		return;
	}

	boost::asio::ip::tcp::endpoint remote_ept(socket_.remote_endpoint());

	std::stringstream ss;
	ss << "receive " << bytes_transferred << " bytes data from: [" << remote_ept.address() << "]:" << remote_ept.port() << "\n";
	utils::out_buffer_in_hex(ss, (unsigned char *)recv_buf, bytes_transferred);
	logger::instance().log(logcollector, log_trace, ss.str());

	if (read_cb_)
		read_cb_(get_id(), recv_buf, bytes_transferred);

	socket_.async_read_some(
		boost::asio::buffer(recv_buf, sizeof(recv_buf)),
		boost::bind(&tcp_server_session::handle_read,
		shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

tcp_server::tcp_server(boost::asio::ip::tcp::endpoint &endpoint, const tcp_server_accept_cb& accept_cb, const tcp_server_read_cb read_cb)
: acceptor_(io_service_, endpoint)
, io_service_()
, logcollector(std::string("tcp-server-")+boost::lexical_cast<std::string>(get_id()), 0x0)
, accept_cb_(accept_cb)
, read_cb_(read_cb)
{
	
}

tcp_server::~tcp_server()
{
}

void tcp_server::start()
{
	tcp_server_session_ptr first_session(new tcp_server_session(io_service_, 
		boost::bind(&tcp_server::handle_read, this, _1, _2, _3), 
		boost::bind(&tcp_server::remove_connect, this, _1)));

	acceptor_.async_accept(first_session->socket(),
		boost::bind(&tcp_server::handle_accept,
		this,
		first_session,
		boost::asio::placeholders::error));

	this->loop_thread.reset(new boost::thread(boost::bind(&tcp_server::run, this)));
}

void tcp_server::stop()
{
	this->acceptor_.close();
	this->loop_thread->join();
	this->loop_thread.reset();
}

bool tcp_server::is_started() const
{
	return acceptor_.is_open();
}

void tcp_server::handle_accept(tcp_server_session_ptr the_session, const boost::system::error_code& error) 
{
	if (error) 
	{
		std::stringstream ss;
		ss << "receive connect with error: " << error << ".";
		logger::instance().log(logcollector, log_error, ss.str());
		return;
	}

	boost::asio::ip::tcp::endpoint remote_ept(the_session->socket().remote_endpoint());
	std::stringstream ss;
	ss << "receive connect from: [" << remote_ept.address() << "]:" << remote_ept.port() << ".";
	logger::instance().log(logcollector, log_trace, ss.str());

	the_session->start();

	{
		boost::mutex::scoped_lock lock(this->sessions_mutex);
		this->sessions[the_session->get_id()] = the_session;
	}

	if (accept_cb_)
		accept_cb_(get_id(), the_session, true);
	
	tcp_server_session_ptr next_session(new tcp_server_session(io_service_, 
		boost::bind(&tcp_server::handle_read, this, _1, _2, _3), 
		boost::bind(&tcp_server::remove_connect, this, _1)));

	acceptor_.async_accept(next_session->socket(),
		boost::bind(&tcp_server::handle_accept,
		this,
		next_session,
		boost::asio::placeholders::error));
}

unsigned int tcp_server::handle_read(int session_id, const unsigned char *data, unsigned int size)
{
	return read_cb_ ? read_cb_(get_id(), session_id, data, size) : 0;
}

void tcp_server::run() 
{
	for (;;)
	{
		try
		{
			io_service_.run();
			break;
		}
		catch (const boost::system::system_error& e)
		{
			std::stringstream ss;
			ss << "tcp server has encountered a system error when running.\n"
				<< "code: " << e.code() << ", what: " << e.what() << "\n";
			logger::instance().log(logcollector, log_fatal, ss.str());
			BOOST_ASSERT(false && ss.str().c_str());
		}
	}
}

void tcp_server::remove_connect(int tcp_server_session_id)
{
	tcp_server_session_ptr the_session;

	{
		boost::mutex::scoped_lock lock(this->sessions_mutex);
		std::map<int, tcp_server_session_ptr>::iterator it = this->sessions.find(tcp_server_session_id);
		if (it != this->sessions.end())
		{
			the_session = it->second;
			this->sessions.erase(it);
		}
	}
	
	if (accept_cb_ && the_session)
		accept_cb_(the_session->get_id(), the_session, false);
}

} // namespace apdu
