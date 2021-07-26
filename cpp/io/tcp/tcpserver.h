
#ifndef __COSIN_TCP_SERVER_H__
#define __COSIN_TCP_SERVER_H__

#include "include.h"
#include "log/logger.h"
#include "apduobject.h"

namespace cosin {

typedef boost::function<unsigned int (int, const unsigned char*, unsigned int)> tcp_server_session_read_cb;
typedef boost::function<void (int)> tcp_server_session_close_cb;
typedef boost::function<unsigned int (int, int, const unsigned char *, unsigned int)>	tcp_server_read_cb;

class tcp_server;

class tcp_server_session : public boost::enable_shared_from_this<tcp_server_session>, public apdu_object
{
public:
	tcp_server_session(boost::asio::io_service &io_service, 
		const tcp_server_session_read_cb& read_cb, 
		const tcp_server_session_close_cb session_close_cb);
	~tcp_server_session();

public:
	void start();
	boost::asio::ip::tcp::socket &socket();
	unsigned int write(const unsigned char *message, unsigned int size);

private:
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

private:
	const log_collector logcollector;
	boost::asio::ip::tcp::socket socket_;
	tcp_server_session_read_cb read_cb_;
	tcp_server_session_close_cb close_cb_;
	unsigned char recv_buf[1024];
};

typedef boost::shared_ptr<tcp_server_session> tcp_server_session_ptr;

typedef boost::function<void (int, tcp_server_session_ptr, bool)> tcp_server_accept_cb;

class tcp_server : public apdu_object
{
	friend class tcp_server_session;
public:
	tcp_server(boost::asio::ip::tcp::endpoint &endpoint, const tcp_server_accept_cb& accept_cb, const tcp_server_read_cb read_cb);
	~tcp_server();

public:
	void start();
	void stop();
	bool is_started() const;

private:
	void handle_accept(tcp_server_session_ptr new_tcp_server_session, const boost::system::error_code& error);
	unsigned int handle_read(int session_id, const unsigned char *data, unsigned int size);
	void run();
	void remove_connect(int tcp_server_session_id);

private:
	const log_collector logcollector;
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::shared_ptr<boost::thread> loop_thread;
	tcp_server_accept_cb	accept_cb_;
	tcp_server_read_cb		read_cb_;
	mutable boost::mutex	sessions_mutex;
	std::map<int, tcp_server_session_ptr>	sessions;
};

typedef boost::shared_ptr<tcp_server> tcp_server_ptr;

} // namespace cosin

#endif // __COSIN_TCP_SERVER_H__
