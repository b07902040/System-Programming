#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(a) { perror(a); exit(1); }

typedef struct {
	char hostname[512];  // server's hostname
	unsigned short port;  // port to listen
	int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
	char host[512];  // client's host
	int conn_fd;  // fd to talk with client
	char buf[512];  // data sent by/to client
	size_t buf_len;  // bytes used by buf
	// you don't need to change this.
	int item;
	int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

typedef struct{
	int id;
	int balance;
} Account;

server svr;  // server 
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

// Forwards

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

static int handle_read(request* reqP);
// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error
int set_rlock ( int fd, int k )  {
	struct  flock  lock ;
	lock.l_whence = SEEK_SET;
	lock.l_start = sizeof(Account)*(k-1) ;
	lock.l_len = sizeof(Account) ;
	lock.l_type = F_RDLCK;
	int p = fcntl(fd, F_SETLK, &lock);
	if(p==0)  return 1;//上鎖
	return 0;//上鎖失敗
}
int set_wlock( int fd, int k ) {
	struct  flock  lock ;
	lock.l_whence = SEEK_SET;
	lock.l_start = sizeof(Account)*(k-1) ;
	lock.l_len = sizeof(Account) ;
	lock.l_type = F_WRLCK;
	int p = fcntl(fd, F_SETLK, &lock);
	if(p==0)  return 1;//上鎖
	return 0;//上鎖失敗
}
void set_ulock( int fd, int k ) {
	struct  flock  lock ;
	lock.l_whence = SEEK_SET;
	lock.l_start = sizeof(Account)*(k-1) ;
	lock.l_len = sizeof(Account) ;
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	return;
}
int main(int argc, char** argv) {
	int i, ret;

	struct sockaddr_in cliaddr;  // used by accept()
	int clilen;

	int conn_fd;  // fd for a new connection with client
	int file_fd;  // fd for file that we open for reading
	char buf[512];
	int buf_len;

	// Parse args.
	if (argc != 2) {
		fprintf(stderr, "usage: %s [port]\n", argv[0]);
		exit(1);
	}

	// Initialize server
	init_server((unsigned short) atoi(argv[1]));

	// Get file descripter table size and initize request table
	maxfd = getdtablesize();
	requestP = (request*) malloc(sizeof(request) * maxfd);
	if (requestP == NULL) {
		ERR_EXIT("out of memory allocating all requests");
	}
	for (i = 0; i < maxfd; i++) {
		init_request(&requestP[i]);
	}
	requestP[svr.listen_fd].conn_fd = svr.listen_fd;
	strcpy(requestP[svr.listen_fd].host, svr.hostname);
	// Loop for handling connections
	fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
	fd_set master, readfds;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_SET(svr.listen_fd, &master);
	maxfd = svr.listen_fd;
	file_fd = open("account_list", O_RDWR);
	int flag[100000] = {0};
	int sinflock[100000] = {0};
	struct flock lock;
	while (1) {
		// TODO: Add IO multiplexing
		readfds = master;
		if(select(maxfd + 1, &readfds, NULL, NULL,&timeout) <= 0)
			continue;
		for(int i = svr.listen_fd; i <= maxfd; i ++) {
			if(FD_ISSET(i, &readfds)){
				if(i == svr.listen_fd){
					clilen = sizeof(cliaddr);
					conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
					if (conn_fd < 0) {
						if (errno == EINTR || errno == EAGAIN) continue;  // try again
						if (errno == ENFILE) {
							(void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
						}
						ERR_EXIT("accept")
					}
					requestP[conn_fd].conn_fd = conn_fd;
					strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
					fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
					FD_SET(conn_fd, &master);
					if(conn_fd > maxfd)
						maxfd = conn_fd;
				}
				else{
					conn_fd = i;
					ret = handle_read(&requestP[conn_fd]); // parse data from client to requestP[conn_fd].buf
					if (ret < 0) {
						fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
						continue;
					}
					int k;
					if(flag[conn_fd]==0)
						k = atoi(requestP[conn_fd].buf);
					#ifdef READ_SERVER
						if( set_rlock(file_fd, k)==1 ) {
							if( sinflock[k]==0) {
								lseek( file_fd, sizeof(int)*2*(k-1), SEEK_SET);
								int *bufi = malloc(sizeof(int));
								read( file_fd, bufi, sizeof(int));
								sprintf(buf,"%d ",*bufi);
								write(requestP[conn_fd].conn_fd, buf, strlen(buf));
								char buf2[200];
								int *bufi2 = malloc(sizeof(int));
								read( file_fd, bufi2, sizeof(int));
								sprintf(buf2,"%d\n",*bufi2);
								write(requestP[conn_fd].conn_fd, buf2, strlen(buf2));
								//close
								set_ulock(file_fd, k);
							}
							else {
								char str[512];
								strcpy(str, "This account is locked!\n");
								write(requestP[conn_fd].conn_fd, str, strlen(str));
							}
						}
						else {
							char str[512];
							strcpy(str, "This account is locked!\n");
							write(requestP[conn_fd].conn_fd, str, strlen(str));
						}
						//close
						flag[conn_fd] = 0;
						FD_CLR(conn_fd, &master);
						close(requestP[conn_fd].conn_fd);
						free_request(&requestP[conn_fd]);
					#else
						if( flag[conn_fd]==0 ) {//第一次輸入登入帳戶
							if( set_wlock(file_fd, k)==1 ) {
								if(sinflock[k]==0) {
									sinflock[k] = 1;
									flag[conn_fd] = 1;
									sprintf(buf,"%s\n","This account is modifiable.");
									write(requestP[conn_fd].conn_fd, buf, strlen(buf));
								}
								else {
									char str[512];
									strcpy(str, "This account is locked!\n");
									write(requestP[conn_fd].conn_fd, str, strlen(str));
									//close
									flag[conn_fd] = 0;
									FD_CLR(conn_fd, &master);
									close(requestP[conn_fd].conn_fd);
									free_request(&requestP[conn_fd]);
								}
							}
							else {
								char str[512];
								strcpy(str, "This account is locked!\n");
								write(requestP[conn_fd].conn_fd, str, strlen(str));
								//close
								flag[conn_fd] = 0;
								FD_CLR(conn_fd, &master);
								close(requestP[conn_fd].conn_fd);
								free_request(&requestP[conn_fd]);
							}
						}
						else if ( flag[conn_fd]==1) {//第二次輸入操作
							int *old_bal = malloc(sizeof(int));
							lseek( file_fd, sizeof(int)*(2*k-1), SEEK_SET);
							read( file_fd, old_bal, sizeof(int));
							if( strncmp(requestP[conn_fd].buf, "save", 4) == 0 ) {
								int* savevalue = malloc(sizeof(int));
								sscanf(&requestP[conn_fd].buf[5], "%d", savevalue);
								if( *savevalue <0 ) {
									char str[512];
									strcpy(str, "Operation failed.\n");
									write(requestP[conn_fd].conn_fd, str, strlen(str));
								}
								else {
									*old_bal += *savevalue;
									lseek(file_fd, sizeof(int)*(2*k-1), SEEK_SET);
									write(file_fd, old_bal, sizeof(int));
								}
							}
							else if (strncmp(requestP[conn_fd].buf, "withdraw", 8) == 0) {
								int  *drawvalue =  malloc(sizeof(int));
								sscanf(&requestP[conn_fd].buf[9], "%d", drawvalue);
								if( (*drawvalue > *old_bal) || (*drawvalue < 0) ) {
									char str[512];
									strcpy(str, "Operation failed.\n");
									write(requestP[conn_fd].conn_fd, str, strlen(str));
								}
								else {
									*old_bal -= *drawvalue;
									lseek(file_fd, sizeof(int)*(2*k-1), SEEK_SET);
									write(file_fd, old_bal, sizeof(int));
								}
							}
							else if (strncmp(requestP[conn_fd].buf, "transfer", 8) == 0) {
								int *reciver = malloc(sizeof(int));
								int *tranvalue = malloc(sizeof(int));
								sscanf(&requestP[conn_fd].buf[9], "%d %d", reciver, tranvalue);
								if( (*tranvalue>*old_bal) || (*tranvalue<0) ) {
									char str[512];
									strcpy(str, "Operation failed.\n");
									write(requestP[conn_fd].conn_fd, str, strlen(str));
								}
								else {
									*old_bal -= *tranvalue;
									lseek(file_fd, sizeof(int)*(2*k-1), SEEK_SET);
									write(file_fd, old_bal, sizeof(int));
									int *reciver_bal = malloc(sizeof(int));
									lseek( file_fd, sizeof(int)*(2*(*reciver)-1), SEEK_SET);
									read( file_fd, reciver_bal, sizeof(int));
									*reciver_bal += *tranvalue;
									lseek( file_fd, sizeof(int)*(2*(*reciver)-1), SEEK_SET);
									write(file_fd, reciver_bal, sizeof(int));
								}
							}
							else if (strncmp(requestP[conn_fd].buf, "balance", 7) == 0) {
								int  *resetvalue =  malloc(sizeof(int));
								sscanf(&requestP[conn_fd].buf[8], "%d", resetvalue);
								if( *resetvalue < 0 ) {
									char str[512];
									strcpy(str, "Operation failed.\n");
									write(requestP[conn_fd].conn_fd, str, strlen(str));
								}
								else {
									*old_bal = *resetvalue;
									lseek(file_fd, sizeof(int)*(2*k-1), SEEK_SET);
									write(file_fd, old_bal, sizeof(int));
								}
							}
							//close
							set_ulock(file_fd, k);
							flag[conn_fd] = 0;
							sinflock[k] = 0;
							FD_CLR(conn_fd, &master);
							close(requestP[conn_fd].conn_fd);
							free_request(&requestP[conn_fd]);
						}	
					#endif
				}
			}
		}
		// Check new connection
	}
	free(requestP);
	return 0;
}
/*flag = 0;
FD_CLR(conn_fd, &master);
close(requestP[conn_fd].conn_fd);
free_request(&requestP[conn_fd]);*/
// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void* e_malloc(size_t size);


static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->item = 0;
    reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error
static int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
	char* p1 = strstr(buf, "\015\012");
	int newline_len = 2;
	// be careful that in Windows, line ends with \015\012
	if (p1 == NULL) {
		p1 = strstr(buf, "\012");
		newline_len = 1;
		if (p1 == NULL) {
			ERR_EXIT("this really should not happen...");
		}
	}
	size_t len = p1 - buf + 1;
	memmove(reqP->buf, buf, len);
	reqP->buf[len - 1] = '\0';
	reqP->buf_len = len-1;
    return 1;
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }
}

static void* e_malloc(size_t size) {
    void* ptr;

    ptr = malloc(size);
    if (ptr == NULL) ERR_EXIT("out of memory");
    return ptr;
}