#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<pthread.h>
#include<iostream>
#include<bits/stdc++.h>
#include <sys/stat.h>
using namespace std;
#define PORT 8080
#define CHUNKSIZE 524288
void* handle_connection(void* p_client_socket);
void* handle_request(string,int ,string&);
string localhost = "127.0.0.1";
vector<string> tokenize(string a);
string findPort(string userId);
class userDetails{
	public:
		string ip;
		int port;
		string psw;
		bool isonline;
		userDetails(){

		}
		userDetails(string i,int po,string pass,bool online){
			ip = i;
			port = po;
			psw = pass;
			isonline = online;
		}
};
class groupDetails{
	public:
		string admin;
		vector<string> users;
		vector<string> files;
		vector<string> pendingUsers;
	groupDetails(){

	}
};
class fileData{
	public:
		int nchunks;
		int lastchunksize;
		vector<pair<string,string>> peerList;
		//peer name,file bitmap....!
		fileData(){
		}
};
map<string,userDetails> userdata;
//userName,userDetails....!
map<string,groupDetails> groupdata;
//groupName,groupDetails...!
map<string,fileData> fileDetails;
//filename,fileDetails...!
int f_id = 1;

int main(int argc, char const *argv[]){
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(localhost.c_str());
	address.sin_port = htons( PORT );
	if (bind(server_fd, (struct sockaddr*) &address,sizeof(address))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 60) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	while(1){
		printf("Waiting for the connection..!\n");
		if( (new_socket = accept(server_fd, (struct sockaddr*) &address,(socklen_t*)(&addrlen))) < 0 ){
			printf("Connection Failed!\n");
			exit(EXIT_FAILURE);
		}else{
			printf("Connected!\n");
		}
		pthread_t t;
		int *pclient = (int*)malloc(sizeof(int));
		*pclient = new_socket;
		pthread_create(&t,NULL,handle_connection,pclient);
		//handle_connection(new_socket);
	}
	return 0;
}
void* handle_connection(void* p_client_socket){
	//printf("Handling connection!\n");
	int client_socket = *((int*)p_client_socket);
	free(p_client_socket);
	char buffer[1024] = {0};
	char hello[1024] = {0};
	string presentUserId;
	while(1){
		int valread = read( client_socket , buffer, 1024);
		string request(buffer);
		handle_request(request,client_socket,presentUserId);
		memset(buffer,0,sizeof(buffer));
	}
}
vector<string> tokenize(string a){
	//cout<<"Tokenize:"<<a<<endl;
	string word = "";
	vector<string> tokens;
	for(int i=0;i<a.size();i++){
		if(a[i]==' '){
			tokens.push_back(word);
			word="";
		}else{
			word += a[i];
		}
	}
	tokens.push_back(word);
	return tokens;
}

void* handle_request(string buffer,int client_socket,string &presentUserId){
	
	cout<<"$~:"<<buffer<<endl;
	vector<string> tokens = tokenize(buffer);
	string reply;
	if(tokens[0] == "create_user"){
		if(tokens.size() == 5){
			//create_user <user_id> <passwd> <port> <ipadr>
			//userdata[tokens[1]]=userDetails(tokens[4],stoi(tokens[3]),tokens[2],false);
			userdata[tokens[1]] = userDetails();
			userdata[tokens[1]].ip = tokens[4];
			userdata[tokens[1]].isonline = false;
			userdata[tokens[1]].port = stoi(tokens[3]);
			userdata[tokens[1]].psw = tokens[2];
			reply.clear();
			reply = "User Created Successfully!";
		}else{
			reply.clear();
			reply = "Invalid command!";
		}
		send(client_socket,reply.c_str(),reply.size(),0);
	}else if(tokens[0] == "login"){
		//Login: login <user_id> <passwd>
		if(tokens.size()==3){
			if(userdata[tokens[1]].psw == tokens[2]){
				reply.clear();
				reply = "Login Sucessful!";
				presentUserId = tokens[1];
				userdata[tokens[1]].isonline = true;
			}else{
				reply.clear();
				reply = "Wrong Password!";
			}
		}else{
			reply.clear();
			reply = "Invalid command!";
		}
		cout<<reply<<endl;
		send(client_socket,reply.c_str(),reply.size(),0);
	}
	else if(tokens[0] == "create_group"){
		//create_group <group_id>
		if(tokens.size()==2){
			groupdata[tokens[1]] = groupDetails();
			groupdata[tokens[1]].admin = presentUserId;
			groupdata[tokens[1]].users.push_back(presentUserId);
			reply.clear();
			reply = "Group created successful!";
		}else{
			reply.clear();
			reply = "Invalid command!";
		}
		send(client_socket,reply.c_str(),reply.size(),0);
	}
	else if(tokens[0] == "join_group"){
		//join_group <group_id>
		groupdata[tokens[1]].pendingUsers.push_back(presentUserId);
		cout<<"Join_group:"<<presentUserId<<endl;
	}
	else if(tokens[0] == "leave_group"){
		//leave_group <group_id>
		groupdata[tokens[1]].users.erase(std::remove(groupdata[tokens[1]].users.begin(),groupdata[tokens[1]].users.end(),presentUserId),groupdata[tokens[1]].users.end());
	}
	else if(tokens[0] == "list_requests"){
		//list_requests <group_id>
		reply.clear();
		for(string s:groupdata[tokens[1]].pendingUsers){
			//cout<<"her:"<<s<<endl;
			reply +=s;
			reply +="$";
		}
		if(reply.size()==0){
			reply.clear();
			reply = "$";
		}
		send(client_socket,reply.c_str(),reply.size(),0);
	}
	else if(tokens[0] == "accept_request"){
		//accept_request <group_id> <user_id>
		groupdata[tokens[1]].pendingUsers.erase(std::remove(groupdata[tokens[1]].pendingUsers.begin(),groupdata[tokens[1]].pendingUsers.end(),tokens[2]),groupdata[tokens[1]].pendingUsers.end());
		groupdata[tokens[1]].users.push_back(tokens[2]);
	}
	else if(tokens[0] == "list_groups"){
		//cout<<"list_groups Here\n";
		reply.clear();
		for(auto i:groupdata){
			//if(i.second.admin != presentUserId){
			reply += i.first;
			reply += "$";
			//}
		}
		//cout<<"Groups:"<<data<<endl;
		if(reply.size()==0){
			reply.clear();
			reply = "$";
		}
		cout<<"Reply:"<<reply<<endl;
		send(client_socket,reply.c_str(),reply.size(),0);
	 }
	else if(tokens[0] == "list_files"){
		//list_files <group_id>
		//vector<string> files = groupdata[tokens[1]].files;
		string data="";
		for(string s: groupdata[tokens[1]].files){
			cout<<"file:"<<s<<endl;
			data += s;
			data += "$";	
		}
		//cout<<"FIles List:"<<data;
		if(data.size()==0){
			data = "$";
		}
		cout<<"data->"<<data<<endl;
		send(client_socket,data.c_str(),data.size(),0);
	}
	else if(tokens[0] == "upload_file"){
		//upload_file <file_name> <group_id> <size>
		fileDetails[tokens[1]] = fileData();
		fileDetails[tokens[1]].nchunks = stoi(tokens[3])/CHUNKSIZE;
		fileDetails[tokens[1]].lastchunksize = stoi(tokens[3])%CHUNKSIZE;
		string bitMap;
		bitMap.append((stoi(tokens[3])/CHUNKSIZE)+1,'1');
		cout<<"Bitmap->"<<bitMap<<endl;
		fileDetails[tokens[1]].peerList.push_back(pair<string,string>(presentUserId,bitMap));
		cout<<"peers"<<endl;
		for(int h = 0;h<fileDetails[tokens[1]].peerList.size();h++){
			cout<<fileDetails[tokens[1]].peerList[h].first<<endl;
		}
		groupdata[tokens[2]].files.push_back(tokens[1]);
		cout<<"file uplded"<<endl;
		for(string h:groupdata[tokens[2]].files){
			cout<<h<<endl;
		}
	}
	else if(tokens[0] == "download_file"){
		//download_file <group_id> <file_name> <destination_path>
		//numberOfPeers$peer1IP peer1PORT peer1BITMAP ..........
		//string fileId = fileloc[tokens[2]];
		reply.clear();
		int npeers = fileDetails[tokens[2]].peerList.size();
		reply += to_string(npeers);
		reply += '$';
		reply += to_string((fileDetails[tokens[2]].nchunks)*CHUNKSIZE+fileDetails[tokens[2]].lastchunksize );
		reply += '$';
		for(int m=0;m<npeers;m++){
			if(userdata[fileDetails[tokens[2]].peerList[m].first].isonline){
				reply += userdata[fileDetails[tokens[2]].peerList[m].first].ip;
				reply += ' ';
				reply += to_string(userdata[fileDetails[tokens[2]].peerList[m].first].port);
				reply += ' ';
				reply += fileDetails[tokens[2]].peerList[m].second;
				reply += ' ';
			}
		}
		cout<<"reply->"<<reply<<endl;
		send(client_socket,reply.c_str(),reply.size(),0);
		//sleep(50);
	}
	else if(tokens[0] == "logout"){
		userdata[presentUserId].isonline = false;
		//presentUserId.clear();
	}
	else if(tokens[0] == "stop_share"){
		//stop_share <group_id> <file_name>

		cout<<"stop_share\n";
	}else if(tokens[0] == "addfile"){
		//addfile username filename nchunks
		string bitMap;
		bitMap.append(stoi(tokens[3])+1,'0');
		fileDetails[tokens[2]].peerList.push_back(pair<string,string>(tokens[1],bitMap));
	}
	else if(tokens[0] == "update_bitmap"){
		//update_bitmap filename chunkNO
		int nofpeer = fileDetails[tokens[1]].peerList.size();
		for(int i=0;i<nofpeer;i++){
			if(fileDetails[tokens[1]].peerList[i].first == presentUserId){
				fileDetails[tokens[1]].peerList[i].second.replace(stoi(tokens[2]),1,"1");
				break;
			}
		}
	}
	else{
		cout<<"Invalid Response!\n";
	}
	
	return NULL;
}
