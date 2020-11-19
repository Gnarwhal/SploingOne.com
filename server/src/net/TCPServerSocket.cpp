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

#include <iostream>

#include "TCPServerSocket.hpp"

auto TCPServerSocket::find_client(u32 id) -> usize {
	auto min = usize( 0 );
	auto max = usize( clients.size() );
	auto mid = (min + max) >> 1;

	while (clients[mid].id != id) {
		if (min + 1 >= max) {
			throw CLIENT_NOT_FOUND;
		}

		if (id > clients[mid].id) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}

	return mid;
}

TCPServerSocket::TCPServerSocket(u32 port) :
	server_socket(::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)),
	current_client_id(0) {
	if (server_socket < 0) {
		throw SOCKET_CREATION_FAILED;
	}

	auto addr            = sockaddr_in{};
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	auto optval = i32(1);
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(i32));

	if (bind(server_socket, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
		throw SOCKET_CREATION_FAILED;
	}

	if (listen(server_socket, 16) < 0) {
		throw SOCKET_CREATION_FAILED;
	}

	server_fd = {};
	server_fd.fd     = server_socket;
	server_fd.events = POLLIN;
}

auto TCPServerSocket::accept(u32 timeout) -> std::vector<u32> {
	auto new_clients = std::vector<u32>();
	auto poll_result = ::poll(&server_fd, 1, timeout);

	while (poll_result > 0 && server_fd.revents & POLLIN) {
		auto socket_address = sockaddr_in{};
		auto address_size   = socklen_t( sizeof(sockaddr_in) );
		auto new_client = Client{
			current_client_id,
			::accept(server_socket, (sockaddr*) &socket_address, &address_size)
		};
		if (new_client.socket >= 0) {
			++current_client_id;
			clients.push_back(new_client);
			new_clients.push_back(new_client.id);

			auto client_fd   = pollfd{};
			client_fd.fd     = new_client.socket;
			client_fd.events = POLLIN | POLLOUT;
			client_fds.push_back(client_fd);
		}
		poll_result = ::poll(&server_fd, 1, 0);
	}

	return new_clients;
}

auto TCPServerSocket::poll_all(u32 timeout) -> std::vector<u32> {
	auto ready_clients = std::vector<u32>();
	auto poll_result   = ::poll(client_fds.data(), client_fds.size(), timeout);

	if (poll_result > 0) {
		for (auto i = 0; i < client_fds.size(); ++i) {
			if (client_fds[i].revents & POLLIN) {
				ready_clients.push_back(clients[i].id);
			}
		}
	}

	return ready_clients;
}

auto TCPServerSocket::poll(u32 client_id, u32 timeout) -> bool {
	auto client_index = find_client(client_id);
	auto poll_result = ::poll(&client_fds[client_index], 1, timeout);

	return poll_result > 0 && client_fds[client_index].revents & POLLIN;
}

auto TCPServerSocket::read(u32 client_id, u32 length) -> std::vector<u8> {
	auto buffer = std::vector<u8>(length);
	recv(clients[find_client(client_id)].socket, buffer.data(), length, 0);
	return buffer;
}

auto TCPServerSocket::read(u32 client_id, u32 length, std::vector<u8> & buffer) -> void {
	recv(clients[find_client(client_id)].socket, buffer.data(), length, 0);
}

auto TCPServerSocket::read_all(u32 client_id, u32 block_size) -> std::vector<u8> {
	auto buffer = std::vector<u8>(block_size);
	recv(clients[find_client(client_id)].socket, buffer.data(), block_size, 0);
	auto offset = 0;
	while (poll(client_id, 0)) {
		buffer.resize(buffer.size() + block_size);
		recv(clients[find_client(client_id)].socket, buffer.data() + (offset += block_size), block_size, 0);
	}
	return buffer;
}

auto TCPServerSocket::read_all(u32 client_id, u32 block_size, std::vector<u8> & buffer) -> void {
	buffer.resize(block_size);	
	recv(clients[find_client(client_id)].socket, buffer.data(), block_size, 0);
	auto offset = 0;
	while (poll(client_id, 0)) {
		buffer.resize(buffer.size() + block_size);
		recv(clients[find_client(client_id)].socket, buffer.data() + (offset += block_size), block_size, 0);
	}
}

auto TCPServerSocket::write_all(std::vector<u8> & bytes) -> void {
	for (auto & client : clients) {
		send(client.socket, bytes.data(), bytes.size(), 0);
	}
}

auto TCPServerSocket::write(u32 client_id, std::vector<u8> & bytes) -> void {
	std::cout << send(clients[find_client(client_id)].socket, bytes.data(), bytes.size(), 0) << std::endl;
}

auto TCPServerSocket::cull_disconnected_clients() -> std::vector<u32> {
	auto disconnected_clients = std::vector<u32>();
	auto poll_result          = ::poll(client_fds.data(), client_fds.size(), 0);

	if (poll_result > 0) {
		for (auto i = 0; i < client_fds.size(); ++i) {
			if (client_fds[i].revents & POLLHUP) {
				disconnected_clients.push_back(clients[i].id);

				::close(clients[i].socket);

				clients.erase(clients.begin() + i);
				client_fds.erase(client_fds.begin() + i);
				--i;
			}
		}
	}

	return disconnected_clients;
}

TCPServerSocket::~TCPServerSocket() {
	for (auto & client : clients) {
		::close(client.socket);
	}

	::close(server_socket);
}
