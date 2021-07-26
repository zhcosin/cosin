
#include "include.h"
#include "tcp/tcpclient.h"

namespace apdu{

	tcp_client::tcp_client(boost::asio::ip::tcp::endpoint &endpoint, const tcp_client_read_cb& read_cb)
		: socket_(io_service_), endpoint_(endpoint), io_service_()
		, read_cb_(read_cb)
		, logcollector(std::string("tcp-client-") + boost::lexical_cast<std::string>(get_id()), 0x0)
	{
		memset(send_buf, 0, sizeof(send_buf));
		memset(recv_buf, 0, sizeof(recv_buf));
	}

	tcp_client::~tcp_client()
	{
	}

	void tcp_client::start() 
	{
		socket_.connect(endpoint_);

		memset(recv_buf, 0, sizeof(recv_buf));
		socket_.async_read_some(boost::asio::buffer(recv_buf, sizeof(recv_buf)),
			boost::bind(&tcp_client::handle_read,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		this->loop_thread.reset(new boost::thread(&tcp_client::loop, this));
	}

	void tcp_client::stop()
	{
		socket_.close();
		this->loop_thread->join();
	}

	bool tcp_client::is_started() const
	{
		return socket_.is_open();
	}

	unsigned int tcp_client::write(const unsigned char *message, unsigned int size)
	{
		if (message == NULL || size == 0)
			throw std::invalid_argument("the message to send is a empty pointer or the length is zero.");

		if (is_started())
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

	void tcp_client::loop()
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
				ss << "tcp client has encountered a system error when running.\n"
					<< "code: " << e.code() << ", what: " << e.what() << "\n";
				logger::instance().log(logcollector, log_fatal, ss.str());
				BOOST_ASSERT(false && ss.str().c_str());
			}
		}
	}

	void tcp_client::handle_read(const boost::system::error_code& error, size_t bytes_transferred) 
	{
		if (error)
		{
			std::stringstream ss;
			ss << "read with error: " << error << " bytes_transferred: " << bytes_transferred << ".";
			logger::instance().log(logcollector, log_error, ss.str());

			return;
		}

		boost::asio::ip::tcp::endpoint remote_ept(socket_.remote_endpoint());

		std::stringstream ss;
		ss << "receive " << bytes_transferred << " bytes data from: [" << remote_ept.address() << "]:" << remote_ept.port() << "\n";
		utils::out_buffer_in_hex(ss, (unsigned char *)recv_buf, bytes_transferred);
		logger::instance().log(logcollector, log_trace, ss.str());

		if (read_cb_)
			read_cb_(get_id(), recv_buf, bytes_transferred);

		memset(recv_buf, 0, sizeof(recv_buf));
		socket_.async_read_some(boost::asio::buffer(recv_buf, sizeof(recv_buf)),
			boost::bind(&tcp_client::handle_read,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
}
