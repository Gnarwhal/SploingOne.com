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

#include "types.hpp"
#include "net/TCPServerSocket.hpp"

i32 main(i32 argc, char ** argv) {
	std::cout << "Hello World!" << std::endl;

	auto socket = TCPServerSocket(0x473);
	auto clients = socket.accept(3 * 60 * 1000);

	for (auto & client : clients) {
		std::cout << "Client connected!" << std::endl;

		auto buffer = socket.read_all(client, 128);
		buffer.push_back('\0');
		std::cout << buffer.data() << std::endl;
		std::cout << "//////////////////////////" << std::endl;

		auto message = std::string("HTTP/1.1 200 Ok\nContent-Type: text/html; charset=utf-8\nContent-Length: 115\n\n<!DOCTYPE html><html><head><meta charset=\"UTF-8\" /><title>Demo</title></head><body><p>Hello World!</p></body></html>");
		auto message_bytes = std::vector<u8>( message.begin(), message.end() );
		socket.write(client, message_bytes);
		message_bytes.push_back('\0');
		std::cout << message_bytes.data() << std::endl;
	}

	return 0;
}

