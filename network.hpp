#pragma once

#include <iostream>
#include <vector>

#include <sys/socket.h>
#include <arpa/inet.h>

template<int SocketType>
class Client {
public:
	Client() : mSocketDescriptor(NOT_SET), mIsOpen(false) {}
	
	bool open(std::string address, int port) {
		if (mSocketDescriptor == NOT_SET) {
			mSocketDescriptor = socket(AF_INET , SocketType , 0);
			
			if (mSocketDescriptor == NOT_SET) {
				return false;
			}
		}
		
		mSocketAddress.sin_addr.s_addr = inet_addr(address.c_str());
		mSocketAddress.sin_family = AF_INET;
		mSocketAddress.sin_port = htons(port);
		
		int result = connect(mSocketDescriptor, (struct sockaddr*) &mSocketAddress, sizeof(mSocketAddress));
		
		mIsOpen = result >= 0;
		return mIsOpen;
	}
	
	bool sendDataString(const std::string &str) {
		
		return send(mSocketDescriptor, str.c_str(), str.size(), 0) > 0;
	}
	
protected:
	enum { NOT_SET = -1 };
	int mSocketDescriptor;
	struct sockaddr_in mSocketAddress;
	bool mIsOpen;
};

using UdpClient = Client<SOCK_DGRAM>;
using TcpClient = Client<SOCK_STREAM>;



template <typename T>
char typeLetter(T value) {
	return 'z';
}

template <> inline char typeLetter(int value) { return 'i'; }
template <> inline char typeLetter(unsigned char value) { return 'i'; }
template <> inline char typeLetter(float value) { return 'f'; }
template <> inline char typeLetter(double value) { return 'f'; }


template <typename T,
typename = typename std::enable_if<std::is_integral<T>::value>::type>
int toOscType(T v) {
	return static_cast<int>(v);
}

template <typename T,
typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
float toOscType(T v) {
	return static_cast<float>(v);
}


class OscSender : public UdpClient {
	
	enum { PADDING_SIZE = 4 };
	
	void addPadding(std::vector<char> &sd, size_t n) {
		for (int i = 0; i < n; i++) {
			sd.push_back('\0');
		}
	}
	
public:
	template<typename T>
	bool send(std::string address, T value) {
		
		// add address
		std::vector<char> stringData(address.begin(), address.end());
		addPadding(stringData, PADDING_SIZE - address.length() % PADDING_SIZE);
		
		// add type tags
		stringData.push_back(',');
		stringData.push_back(typeLetter(value));
		addPadding(stringData, 2);
		
		// add data
		// TODO: check endianess
		auto oscValue = toOscType(value);
		auto *bytes = reinterpret_cast<char const *>(&oscValue);
		for (int i = 0; i < 4; i++) {
			stringData.push_back(bytes[3-i]);
		}
		
		std::string dataString(stringData.begin(), stringData.end());
		return Client<SOCK_DGRAM>::sendDataString(dataString);
	}
};

