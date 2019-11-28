#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

const char * RESPONSE_TEMPLATE =
"HTTP/1.1 %s\n"
"Content-Type: %s\n"
"Content-Length: %d\n"
"\n";

char * copy(const char * str) {
	int length = -1;
	while (str[++length]);
	char * ret = malloc(length + 1);
	char * iter = ret - 1;
	--str;
	while (*++iter = *++str);
	ret[length] = '\0';
	return ret;
}

char * substring(const char * begin, const char * end) {
	int length = end - begin;
	char * ret = malloc(length + 1);
	for (int i = 0; i < length; ++i)
		ret[i] = begin[i];
	ret[length] = '\0';
	return ret;
}

int compare(const char * str1, const char * str2) {
	--str1;
	--str2;
	while (*++str1 == *++str2 && *str1 && *str2);
	return !(*str1 || *str2);
}

int subcompare(const char * str1, const char * str2, int len) {
	for (int i = 0; i < len; ++i) {
		if (str1[i] != str2[i]) {
			return 0;
		}
		else if(!str1[i]) {
			return i == len - 1;
		}
	}
	return 1;
}

void processRequest(const char * request, char ** responseCode, char ** type, char ** filePath) {
	if (!subcompare(request, "GET", 3)) {
		*responseCode = copy("405 Method Not Allowed");
		*type         = copy("text/html; charset=utf-8");
		*filePath     = copy("invalid_method.html");

		printf("Invalid method!\n");
		return;
	}

	const char * begin = request;
	while (*++begin != '\n');
	while (*--begin != '/');
	if (!subcompare(++begin, "1.1", 3)) {
		*responseCode = copy("505 HTTP Version Not Supported");
		*type         = copy("text/html; charset=utf-8");
		*filePath     = copy("unsupported_version.html");
		
		printf("Unsupported http version!\n");
		return;
	}
	
	begin = request;
	while (*++begin != '/');
	const char * end = begin;
	while (*++end != ' ');

	if (subcompare(begin, "/ ", 2)) {
		*filePath = copy("index.html");
	}
	else {
		*filePath = substring(begin + 1, end);
	}

	FILE * file = fopen(*filePath, "r");
	if (file == 0) {
		free(*filePath);

		*responseCode = copy("404 Not Found");
		*type         = copy("text/html; charset=utf-8");
		*filePath     = copy("not_found.html");

		printf("Resource not found!\n");
		return;
	}
	fclose(file);

	end = *filePath + strlen(*filePath);;
	const char * extensionBegin = end;
	while (--extensionBegin > *filePath && *extensionBegin != '.');

	if (extensionBegin == *filePath || extensionBegin + 1 == end) {
		free(*filePath);

		*responseCode = copy("415 Unsupported Media Type");
		*type         = copy("text/html; charset=utf-8");
		*filePath     = copy("unsupported_media_type.html");

		printf("Unsupported media type!\n");
		return;
	}

	char * extension = substring(extensionBegin + 1, end);
	if (compare(extension, "html")) {
		*responseCode = copy("200 OK");
		*type         = copy("text/html; charset=utf-8");
	}
	else if (compare(extension, "jpg") || compare(extension, "jpeg")) {
		*responseCode = copy("200 OK");
		*type         = copy("image/jpeg");
	}
	else if (compare(extension, "png")) {
		*responseCode = copy("200 OK");
		*type         = copy("image/png");
	}
	else if (compare(extension, "gif")) {
		*responseCode = copy("200 OK");
		*type         = copy("image/gif");
	}
	else if (compare(extension, "ico")) {
		*responseCode = copy("200 OK");
		*type         = copy("image/x-icon");
	}
	else if (compare(extension, "js")) {
		*responseCode = copy("200 OK");
		*type         = copy("application/javascript");
	}
	else if (compare(extension, "css")) {
		*responseCode = copy("200 OK");
		*type         = copy("text/css");
	}
	else {
		free(*filePath);

		*responseCode = copy("415 Unsupported Media Type");
		*type         = copy("text/html; charset=utf-8");
		*filePath     = copy("unsupported_media_type.html");
		
		printf("Unsupported media type!\n");
	}
	free(extension);
	printf("Successfully processed request!\n");
}

void sendResponse(int clientSocket, const char * request) {	
	char * responseCode;
	char * type;
	char * filePath;

	processRequest(request, &responseCode, &type, &filePath);
	
	FILE * file = fopen(filePath, "r");
	
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);	

	char buffer[2048];
	sprintf(buffer, RESPONSE_TEMPLATE, responseCode, type, fileSize);

	free(responseCode);
	free(type);
	free(filePath);

	send(clientSocket, buffer, strlen(buffer), 0);

	fseek(file, 0, SEEK_SET);
	int bytesRead = 0;
	while ((bytesRead = fread(buffer, 1, 2048, file)) > 0) {
		send(clientSocket, buffer, bytesRead, 0);
	}
	fclose(file);
}

int main(int argc, char ** argv) {
	int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 1) {
		printf("Failed to create socket!\n");
		return -1;
	}

	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(struct sockaddr_in));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(80);

	int optval = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

	if (bind(serverSocket, (const struct sockaddr *) &serverAddress, sizeof(struct sockaddr_in)) < 0) {
		printf("Failed to bind socket!\n");
		return -1;
	}

	if (listen(serverSocket, 5) < 0) {
		printf("Failed to start listening!");
		return -1;
	}

	unsigned int socketAddressLength;
	int clientAddressLength = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddress;
	for (;;) {
		int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);

		char buffer[1024];
		int bytesRead = recv(clientSocket, buffer, 1023, 0);
		buffer[bytesRead] = '\0';
		printf("%s", buffer);

		printf("--------------------------------------\n");

		sendResponse(clientSocket, buffer);	

		printf("--------------------------------------\n\n");
		
		shutdown(clientSocket, SHUT_RDWR);
		close(clientSocket);
	}

	close(serverSocket);

	return 0;
}
