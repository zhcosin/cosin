
#ifndef __LIBAPDU_TCPCLIENT_H__
#define __LIBAPDU_TCPCLIENT_H__

#include "include.h"
#include "utils/utils.h"
#include "log/logger.h"
#include "apduobject.h"

namespace apdu{

	typedef boost::function<unsigned int (int, const unsigned char *, unsigned int)>	tcp_client_read_cb;

	class tcp_client : public boost::enable_shared_from_this<tcp_client>, public apdu_object
	{
	public:
		tcp_client(boost::asio::ip::tcp::endpoint &endpoint, const tcp_client_read_cb& read_cb);
		~tcp_client();

		void start();

		void stop();

		bool is_started() const;

		unsigned int write(const unsigned char *message, unsigned int size);

	private:
		void loop();

		void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

	private:
		const log_collector logcollector;
		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint endpoint_;
		unsigned char send_buf[1024];
		unsigned char recv_buf[1024];
		boost::shared_ptr<boost::thread> loop_thread;
		tcp_client_read_cb		read_cb_;
	};

	typedef boost::shared_ptr<tcp_client> tcp_client_ptr;

}

#endif
