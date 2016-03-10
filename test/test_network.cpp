#include <iostream>
#include "network.h"

using namespace std;

int main(int argc, const char * argv[]) {
	// insert code here...
	std::cout << "Hello, World!\n";
	std::cout << std::boolalpha;
	
	OscSender client;
	
	bool opened = client.open("127.0.0.1", 9000);
	cout << "opened = " << opened << endl;
	//	bool sent = client.sendString("asdf");
	//	cout << "sent = " << sent << endl;
	
	string input;
	cin >> input;
	while (input != "q") {
		//		bool opened = client.open("127.0.0.1", 9009);
		//		cout << "opened = " << opened << endl;
		cout << "sending " << input << endl;
		//		bool sent = client.sendDataString(input);
		bool sent = client.send("/a", 3.14);
		cout << "sent = " << sent << endl;
		cin >> input;
	}
	
	return 0;
}