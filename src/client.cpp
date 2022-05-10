#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>
#include<bits/stdc++.h>
#include<pthread.h>
#include <thread>
#include <sys/stat.h>
using namespace std;
#define PORT 8080
#define CHUNKSIZE 524288
string localhost = "127.0.0.7";
void* startserver(void* arg);
int getport(const char* a);
void* handle_request(string request, int conn_socket);
void* handle_download(void* message);
void* handle_peer(void* peer_socket);
//void* get_chunk_from(void* req_obj);
vector<string> tokenize(string a);
char* receive_from_server(int fd, int SIZE, int sz_received, int* length, int* read_err);
struct reqpeer{
	//abc.txt$8081$filesize$destinationLocation
	string filename;
	int filesize;
	string destination_location;
	vector<pair<string,int>> chunk_peer_no;
};
void* get_chunk_from(struct req_chunk req_obj);
struct req_chunk{
	int chunk_no;
	string ip_addr;
	int port_no;
	string file_name;
	string desti_loc;
	int chunk_size;
};

class fileData{
	public:
		string file_location;
		int n_chunks;
		int last_chunk_size;
		string bitMap;
		int file_size;
		fileData(){

		}
};
struct server{
	int port;
	string ip_addr;
};
map<string,fileData> filedetails;
//filename,fileData..!
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

int getport(string aa){
	//string aa(a);
	int pos = aa.find(":");
	return stoi(aa.substr(pos+1));
}
string getip(string aa){
	int pos = aa.find(":");
	return aa.substr(0,pos);
}
int main(int argc, char **argv)
{	
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *buffer;
	//[1024] = {0};
	buffer = (char*)malloc(sizeof(char)*1024);
	//if(strcmp(sendb,"peer")==0){	
			pthread_t serve;
			string arg(argv[1]);
			//string arg = "";
			//cout<<argv[1];
			struct server ser;// = (struct server*)malloc(sizeof(server));
			(ser).port = getport(arg);
			ser.ip_addr = getip(arg);
			//cout<<"peer to peer:"<<serveport<<endl;
			pthread_create(&serve,NULL,startserver,(void*)&ser);
			//break;
	//}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	char* sendb;
	size_t maxsize = 1024;
	
	sendb = (char *)malloc(maxsize * sizeof(char));
	while(1){
		memset(sendb,0,1024);
		printf("$:");
		//scanf("%s",sendb);
		getline(&sendb,&maxsize,stdin);
		string msg(sendb);
		sendb[strlen(sendb)-1] = '\0';
		if(msg.find("create_user",0) != string::npos){
			//msg = msg + " " + to_string(serveport);
			//sendb = msg.c_str();
			char space[10] = " ";
			std::strcat(sendb,space);
			char portchar[10];
			sprintf(portchar,"%d",(ser).port);
			std::strcat(sendb,portchar);
			std::strcat(sendb,space);
			std::strcat(sendb,(ser).ip_addr.c_str());
		}
		if(msg.find("upload_file",0) != string::npos){
			/////todo something
			//add file size and manage path..!
			//upload_file <file_path> <group_id> <size>
			msg.pop_back();
			vector<string> toks = tokenize(msg);
			struct stat file_stat;
			stat(toks[1].c_str(),&file_stat);
			int f_s = file_stat.st_size;
			string f_n;//file name
			string f_l;//file location
			string bitMap;
			char tem[256];
            getcwd(tem,256);
            string address(tem);
			size_t found = toks[1].find('/');
			if (found == string::npos){
				f_n = toks[1];
				f_l = address + '/' + toks[1];
			}else{
				size_t found = toks[1].find_last_of("/\\");
				f_n = toks[1].substr(found+1);
				f_l = toks[1];
			}
        	bitMap.append((f_s/CHUNKSIZE)+1,'1');
			filedetails[f_n] = fileData();
			filedetails[f_n].bitMap = bitMap;
			filedetails[f_n].last_chunk_size = f_s % CHUNKSIZE;
			filedetails[f_n].file_location = f_l;
			filedetails[f_n].n_chunks = (f_s/CHUNKSIZE)+1;
			filedetails[f_n].file_size = f_s;
			msg = "upload_file " + f_n + " " + toks[2] + " " + to_string(f_s);
			memset(sendb,0,1024);
			for(int k=0;k<msg.size();k++){
				sendb[k] = msg[k];
			}
		}
		//if(strlen(sendb) > 3)
			//cout<<"sending"<<endl;
			if(send(sock , sendb , strlen(sendb) , 0 )<0)
				printf("Send Error!\n");
		if(msg.find("create_user",0) != string::npos){
			valread = read( sock , buffer, 1024);
			cout<<string(buffer)<<endl;
			memset(buffer,0,1024);
		}
		if(msg.find("login",0) != string::npos){
			valread = read( sock , buffer, 1024);
			cout<<string(buffer)<<endl;
			memset(buffer,0,1024);
		}
		if(msg.find("create_group",0) != string::npos){
			valread = read( sock , buffer, 1024);
			cout<<string(buffer)<<endl;
			memset(buffer,0,1024);
		}
		if(msg.find("list_requests",0) != string::npos){
			valread = read( sock , buffer, 1024);
			string msg1(buffer);
			//cout<<endl<<msg1<<endl;
			//cout<<"Pending requests are:";
			string word = "";
			vector<string> tokens;
			for(int i=0;i<msg1.size();i++){
				if(msg1[i]=='$'){
					cout<<word<<endl;
					word="";
				}else{
					word += msg1[i];
				}
			}
			memset(buffer,0,1024);
		}
		if(msg.find("list_groups",0) != string::npos){
			valread = read( sock , buffer, 1024);
			string msg1(buffer);
			//cout<<msg1<<":Groups are:";
			string word = "";
			for(int i=0;i<msg1.size();i++){
				if(msg1[i]=='$'){
					cout<<word<<endl;
					word="";
				}else{
					word += msg1[i];
				}
			}
			memset(buffer,0,1024);
		}
		if(msg.find("list_files",0) != string::npos){
			valread = read( sock , buffer, 1024);
			string msg1(buffer);
			int count=0;
			//cout<<"Files are:\n";
			string word = "";
			for(int i=0;i<msg1.size();i++){
				if(msg1[i]=='$'){
					cout<<word<<"--";
					word="";
					count++;
				}else{
					word += msg1[i];
				}
			}
			cout<<endl;
			memset(buffer,0,1024);
		}if(msg.find("download_file",0) != string::npos){
			valread = read( sock , buffer, 1024);
			for(int d=0;d<strlen(buffer);d++){
				cout<<buffer[d];
			}cout<<endl;
			//download_file <group_id> <file_name> <destination_path> 
			vector<string> send_toks = tokenize(string(sendb));
			if(send_toks[3] == "."){
				char tem[256];
            	getcwd(tem,256);
            	string address(tem);
				cout<<"address:"<<address<<endl;
				send_toks[3] = address+"/copy_"+send_toks[2];
			}else{
				send_toks[3] = send_toks[3] + '/' + send_toks[2];
			}
			string mesg = string(buffer);
			size_t br = mesg.find('$');
			size_t br1 = mesg.find('$',br+1);
			int npeers = stoi(mesg.substr(0,br));
			int f_s = stoi(mesg.substr(br+1,br1-br-1));
			vector<string> toks = tokenize(mesg.substr(br1+1));
			string iparray[npeers];
			string portarray[npeers];
			string bmarray[npeers];
			int w = 0;
			for(int l=0;l<npeers;l++){
				iparray[l] = toks[w++];
				portarray[l] = toks[w++];
				bmarray[l] = toks[w++];
			}
			vector<vector<pair<string,int>>>file_chunk_info((f_s/CHUNKSIZE) + 1);
			for(int r=0;r<((f_s/CHUNKSIZE) + 1);r++){
				for(int q=0;q<npeers;q++){
					if(bmarray[q][r] == '1'){
						file_chunk_info[r].push_back(pair<string,int>(iparray[q],stoi(portarray[q])));
					}
				}
			}
			vector<pair<string,int>>chunk_from_peer(((f_s/CHUNKSIZE) + 1));
			for(int r=0;r<((f_s/CHUNKSIZE) + 1);r++){
				if(file_chunk_info[r].size() == 1){
					chunk_from_peer[r] = file_chunk_info[r][0];
				}else{
					int num_peers = file_chunk_info[r].size();
					int rand_loc = ( rand() )% num_peers ;
					chunk_from_peer[r] = file_chunk_info[r][rand_loc];
				}
			}
			//-----------------------create_file--------------------------------------------//
			//FILE *fp = fopen(send_toks[3].c_str(), "a");
			// ftruncate(fileno(fp),f_s);
			int nc = f_s / CHUNKSIZE;
			int lcs = f_s % CHUNKSIZE;
			fstream out;
			out.open(send_toks[3].c_str(),ios::out|ios::binary);
			for(int w=0;w<nc;w++){
				string dum(CHUNKSIZE,'\0');
				out.write(dum.c_str(),CHUNKSIZE);
			}
			string dum1(lcs,'\0');
			out.write(dum1.c_str(),lcs);
			out.close();
			// fseek(fp, f_s-1 , SEEK_SET);
        	// fputc('\0', fp);
			// fclose(fp);
			cout<<"Dummy File Created:"<<endl;
			struct reqpeer *req;
			req = (reqpeer *)malloc(sizeof(reqpeer));
			(*req).chunk_peer_no = chunk_from_peer;
			(*req).destination_location = send_toks[3];
			(*req).filename = send_toks[2];
			(*req).filesize = f_s;
			pthread_t start_down;
			// size_t br1 = mesg.find('$',br+1);
			// size_t b2 = mesg.find('$',br1+1);
			// cout<<mesg.substr(0,br)<<endl;
			// string f = mesg.substr(0,br);
			// (*req).filename = f;
			// string di = mesg.substr(b2+1);
			// (*req).destination_location = di;
			// (*req).filesize = stoll(mesg.substr(br1+1,b2-br1-1));
			// (*req).portnum = stoi(mesg.substr(br+1,br1-br-1));
			pthread_create(&start_down,NULL,handle_download,(void*)req);
			
			memset(buffer,0,1024);
		}
		memset(sendb,0,1024);
	}
	
	return 0;
}


void* handle_download(void* re){
	string fileName,dest_loc;
	int filesize;
	vector<pair<string,int>> chunk_peer;
	struct reqpeer *req = (struct reqpeer*)re;
	fileName = (*req).filename;
	filesize = (*req).filesize;
	dest_loc = (*req).destination_location;
	chunk_peer = (*req).chunk_peer_no;
	int nchunks = ( filesize / CHUNKSIZE )+1;

	cout<<"\nFileName:"<<fileName<<endl;
	cout<<"FileSize:"<<filesize<<endl;
	cout<<"Destination Location:"<<dest_loc<<endl;
	cout<<"chunks from:"<<chunk_peer.size()<<endl;
	for(int g=0;g<chunk_peer.size();g++){
		cout<<g<<"th "<<chunk_peer[g].first<<":"<<chunk_peer[g].second<<endl;
	}
	/*
	pthread_t get_chunk[nchunks];
	struct req_chunk *rp = (struct req_chunk*)malloc(sizeof(req_chunk));
	for(int i=0;i<chunk_peer.size();i++){
		cout<<i<<" ";
		
		(*rp).chunk_no = i;
		cout<<"+"<<endl;
		(*rp).desti_loc = dest_loc;
		cout<<"-"<<endl;
		(*rp).file_name = fileName;
		cout<<"*"<<endl;
		//(*rp).ip_addr = 
		rp->ip_addr = chunk_peer[i].first;
		cout<<"/"<<endl;
		(*rp).port_no = chunk_peer[i].second;
		cout<<"="<<endl;
		if(i==chunk_peer.size()-1){
			(*rp).chunk_size = filesize % CHUNKSIZE;
		}else{
			(*rp).chunk_size = CHUNKSIZE;
		}
		cout<<"#"<<endl;
		//pthread_create(&get_chunk[i],NULL,get_chunk_from,(void*)rp);
		get_chunk_from((void*)rp);
		cout<<"returnned!"<<endl;
	}
	free(rp);*/
	struct req_chunk rp;
	for(int i=0;i<chunk_peer.size();i++){
		rp.chunk_no = i;
		rp.desti_loc = dest_loc;
		rp.file_name = fileName;
		rp.ip_addr = chunk_peer[i].first;
		rp.port_no = chunk_peer[i].second;
		if(i==chunk_peer.size()-1){
			rp.chunk_size = filesize % CHUNKSIZE;
		}else{
			rp.chunk_size = CHUNKSIZE;
		}
		get_chunk_from(rp);
	}
	for(int i=0;i<nchunks;i++){
		//pthread_join(get_chunk[i],NULL);
	}
	return NULL;

}
//void* get_chunk_from(void* r_o){
void* get_chunk_from(struct req_chunk req_obj){
		//cout<<"here1"<<endl;
	// struct req_chunk *req_obj = (struct req_chunk*) r_o;
	// int c_no = (*req_obj).chunk_no;
	// string ip_addr = (*req_obj).ip_addr;
	// int port_number = (*req_obj).port_no;
	// string file_name = (*req_obj).file_name;
	// string dest_location = (*req_obj).desti_loc;
	// int c_size = (*req_obj).chunk_size; 
	int c_no = (req_obj).chunk_no;
	string ip_addr = (req_obj).ip_addr;
	int port_number = (req_obj).port_no;
	string file_name = (req_obj).file_name;
	string dest_location = (req_obj).desti_loc;
	int c_size = (req_obj).chunk_size; 
	//-------------------Starting client code--------------------------------------//
	int peerfd;
	struct sockaddr_in peer_server;
	peerfd = socket(AF_INET,SOCK_STREAM,0);
	if(peerfd < 0){
		perror("[-]Error in socket");
		return NULL;
	}
	peer_server.sin_family = AF_INET;
	peer_server.sin_port = htons(port_number);
	//peer_server.sin_addr.s_addr = INADDR_ANY;
	peer_server.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	int e = connect(peerfd,(struct  sockaddr*)&peer_server,sizeof(peer_server));
	if(e==-1){
		perror("[-]Error in socket");
		return NULL;
	}
	//send_file file_name CHunk_no chunk_size 
	string content = "send_file "+file_name+' '+to_string(c_no) + ' ' + to_string(c_size);

	if(send(peerfd , content.c_str(), content.size() , 0 )<0){
		printf("Send Command Error!\n");
		return NULL;
	}

	//------------------------------Recieving file------------------------------------------//
	char filecontent[c_size] = {0};
	fstream dest(dest_location.c_str(), std::fstream::in|std::fstream::out  |std::fstream::binary);
	//int y = read(peerfd,filecontent,c_size);
	// int offset = 0,y;
	// while(1){
	// 	y = read(peerfd,filecontent+offset,c_size);
	// 	cout<<"a"<<y<<endl;
	// 	if(y<=0){
	// 		break;
	// 	}
	// 	offset += y;
	// }
	int length = 0,read_err=0;
	char* filecon = receive_from_server(peerfd,c_size,1024,&length,&read_err);
	cout<<"Recievied chunk:"<<c_no<<" "<<length<<endl;
	dest.seekp(c_no*CHUNKSIZE,ios::beg);
	dest.write(filecon,length);
	bzero(filecon,c_size);
	dest.close();
	close(peerfd);
	return NULL;
}
char* receive_from_server(int fd, int SIZE, int sz_received, int* length, int* read_err){
  *read_err = 0;
  int i = 0, sz = 0, rt = 0, count=0;
  char *array = (char *)malloc(SIZE);
  memset(array, 0, SIZE);  
  for (i = 0; i < SIZE; i += sz_received){
      while(sz_received-sz){ 
        rt = read(fd, array + i + sz, sz_received-sz);
        if(rt==-1){
          *read_err=rt;
          printf("an error occurs\n");
          goto l;
        }
        if(!rt)goto l;
        sz+=rt;
        count += sz;   
      }
      sz = 0;
    }
  l: *length = count;
  return array;
}

void* startserver(void* argv){
	struct server *serve = (struct server*)argv;
	int port = (*serve).port;
	string ip = (*serve).ip_addr;
	//cout<<port<<":Port Num:";
	int serve_fd,conn_socket;
	struct sockaddr_in socaddr;
	int addrlen = sizeof(socaddr);
	int option = 1;
	if((serve_fd = socket(AF_INET,SOCK_STREAM,0))==0){
		perror("Socket Failed!");
		return NULL;
	}
	if(setsockopt(serve_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof(option))){
		perror("setsockopt");
		return NULL;
	}
	socaddr.sin_family = AF_INET;
	socaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	//socaddr.sin_addr.s_addr = INADDR_ANY;
	socaddr.sin_port = htons(port);

	if (bind(serve_fd, (struct sockaddr *)&socaddr,sizeof(socaddr))<0)
    {
        perror("bind failed");
        return NULL;
    }
    if (listen(serve_fd, 100) < 0)
    {
        perror("listen");
        return NULL;
    }
	while(1){
		if ((conn_socket = accept(serve_fd, (struct sockaddr *)&socaddr, (socklen_t*)&addrlen))<0){
			perror("accept");
			return NULL;
    	}
		pthread_t newPeer;
		int *connect_socket = &conn_socket;
		pthread_create(&newPeer,NULL,handle_peer,connect_socket);

	}
    
	return NULL;
}

void* handle_peer(void* peer_socket){
	char buff[1024];
	char hello[1024];
	int conn_socket = *((int*)peer_socket);
	while(1){
		int z = read( conn_socket , buff, 1024);
		//printf("valread:%d\n",valread);
		if(z<=0)break;
		// printf("\npeer>>%s\n",buff );
		// cout<<"\n$:";
		string request(buff);
		handle_request(request,conn_socket);	
		memset(buff,0,sizeof(buff));
	}
	return NULL;
}
void write_to_client(int fd, char* array, int SIZE, int sz_emit){
   int i=0, sz=0;
   for(i = 0; i < SIZE; i += sz_emit ){  
       while(sz_emit-sz){ 
         sz+=write(fd, array+i+sz, sz_emit-sz);
       }
       sz = 0;
   }
   return;
 }

void* handle_request(string request,int conn_socket){
	////send_file file_name CHunk_no chunk_size 
	vector<string> req_tok = tokenize(request);
	if(req_tok[0] == "send_file"){
		string file_name = req_tok[1];
		int chunk_number = stoi(req_tok[2]);
		int chunk_size = stoi(req_tok[3]);
		char filedata[chunk_size] = {0};
		ifstream f;
		int i;
		//-------------------------------------------sending chunk file-----------------------//
		f.open(filedetails[file_name].file_location.c_str(),ios::in|ios::binary);
		f.seekg(chunk_number*CHUNKSIZE,ios::beg);
		f.read(filedata,chunk_size);
		//write(conn_socket,filedata,sizeof(filedata));
		write_to_client(conn_socket,filedata,sizeof(filedata),1024);
		cout<<"sent:"<<chunk_number<<" "<<chunk_size<<endl;
		bzero(filedata,chunk_size);
		f.close();
	}
	return NULL;
}
