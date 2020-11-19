/*******************************************************************************
 *
 * Copyright (c) 2020 Gnarwhal
 *
 * -----------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#ifndef GNARWHAL_WEBSERVER_TCP_SERVER_SOCKET
#define GNARWHAL_WEBSERVER_TCP_SERVER_SOCKET

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#include <vector>

#include "../types.hpp"

class TCPServerSocket {
	private:
		struct Client {
			u32 id;
			i32 socket;
		};

		i32    server_socket;
		pollfd server_fd;

		u32 current_client_id;
		std::vector<Client> clients;
		std::vector<pollfd> client_fds;

		auto find_client(u32 id) -> usize;

	public:
		const static auto SOCKET_CREATION_FAILED = u32( 0x00 );
		const static auto CLIENT_NOT_FOUND       = u32( 0x01 );

		TCPServerSocket(u32 port);

		auto accept(u32 timeout) -> std::vector<u32>;
		
		auto poll_all(u32 timeout) -> std::vector<u32>;
		auto poll(u32 client_id, u32 timeout) -> bool;
		auto read(u32 client_id, u32 length) -> std::vector<u8>;
		auto read(u32 client_id, u32 length, std::vector<u8> & buffer) -> void;
		auto read_all(u32 client_id, u32 block_size) -> std::vector<u8>;
		auto read_all(u32 client_id, u32 block_size, std::vector<u8> & buffer) -> void;

		auto write_all(std::vector<u8> & bytes) -> void;
		auto write(u32 client, std::vector<u8> & bytes) -> void;

		auto cull_disconnected_clients() -> std::vector<u32>;
		
		~TCPServerSocket();
};

#define GNARWHAL_WEBSERVER_TCP_SERVER_SOCKET_FORWARD
#endif // GNARWHAL_WEBSERVER_TCP_SERVER_SOCKET

