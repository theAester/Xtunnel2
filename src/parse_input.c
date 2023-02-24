#include "messaging.h"
#include "parse_input.h"

char* progname;

int print_usage(int type){
	fprintf((type==1 ? stderr : stdout), 
		 "Usage\n"
		 "\t%s [-h] [-l] -u client_addr:client_port@server_addr:server_port[-...] -s xserver_addr:xserver_port listen_port_no\n"
		 "\n"
		 "listen_port_no: the port that xclient will be listening to\n"
		 "\n"
		 "\t-l\t:\tlisten. makes this end act as the Xserver.\n"
		 "\t\t\tby default if this flag is not specifies, this\n"
		 "\t\t\tend acts as Xclient.\n"
		 "\t\t\tat the presence of this flag, xserver_addr is\n"
		 "\t\t\tignored\n"
		 "\n"
		 "\t-h\t:\tdisplays this help message\n"
		 "\n"
		 "\t-u\t:\tclientlist. enter each entry in the specified\n"
		 "\t\t\tformat separated by dashes(-) do not\n"
		 "\t\t\tplace a dash at the end of the list!\n"
		 "\n"
		 "\t-s\t:\taddress and port of the\n"
		 "\t\t\txserver process\n\n"
		 , progname
	);
	return 0;
}

int extract_xserver(char* str, struct sockaddr_in* xserveraddr){
	char* portstr = strchr(str, ':');
	if(portstr == NULL || (int)(portstr-str) + 1 >= strlen(str)){
		fprintf(stderr, "xserveraddr: where port?\n");
		return 1;
	}
	char* endstr;
	int temp = strtol(portstr+1, &endstr, 10);
	if(!(strcmp(portstr, "") && *endstr == '\0')){
		fprintf(stderr, "xserveraddr: bad port number; %s is not numeric\n", portstr);
		return 1;
	}
	*portstr = '\0';
	xserveraddr->sin_family = AF_INET;
	xserveraddr->sin_port = htons(temp);
	if(str == portstr){
		inet_pton(AF_INET, "127.0.0.1", &(xserveraddr->sin_addr));
	}else if(inet_pton(AF_INET, str, &(xserveraddr->sin_addr)) == 0){
		fprintf(stderr, "xserveraddr: bad ip address %s\n", str);
		fflush(stderr);
		*portstr = ':';
		return 1;
	}
	*portstr = ':';
	return 0;
}

int extract_client(char* str, struct client_list** client_list, int* list_len){
	char* seek = str;
	enum {
		CLIENTADDR,
		CLIENTPORT,
		SERVERADDR,
		SERVERPORT
	} state;
	state = CLIENTADDR;
	int proceed = 1;
	int ind = 0;
	struct client_list* list = NULL;
	struct client_list* newelem;
	char* endstr;
	int temp;
	while(proceed){
		char* pos;
		switch(state){
			case CLIENTADDR:
				pos = strchr(seek, ':');
				if(pos == NULL){
					fprintf(stderr, "clientaddr_list::clientaddr: missing \':\'\n");
					return 1;
				}
				*pos = '\0';
				list = (struct client_list*)realloc((void*)list, (ind+1)*sizeof(struct client_list));
				newelem = list + ind;
				newelem->client = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
				newelem->server = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
				memset(newelem->client, 0, sizeof(struct sockaddr_in));
				memset(newelem->server, 0, sizeof(struct sockaddr_in));
				newelem->client->sin_family = AF_INET;
				if(pos == seek){
					inet_pton(AF_INET, "127.0.0.1", &(newelem->client->sin_addr));
				}else if(inet_pton(AF_INET, seek, &(newelem->client->sin_addr)) == 0){
					fprintf(stderr, "clientaddr_list::clientaddr: invalid ip address %s\n", seek);
					fflush(stderr);
					*pos = ':';
					return 1;
				}
				*pos = ':';
				state = CLIENTPORT;
				seek = pos +1;
			break;
			case CLIENTPORT:
				pos = strchr(seek, '@');
				if(pos == NULL){
					fprintf(stderr, "clientaddr_list::clientaddr: missing \'@\'");
					return 1;
				}
				if((int)(pos-seek) < 2){
					fprintf(stderr, "clientaddr_list::clientaddr: where port?\n");
					return 1;
				}
				*pos = '\0';
				temp = strtol(seek, &endstr, 10);
				if(!(strcmp(seek, "") && *endstr == '\0')){
					fprintf(stderr, "clientaddr_list::clientaddr: bad port number; %s is not numeric\n", seek);
					return 1;
				}
				*pos = '@';
				newelem->client->sin_port = htons(temp);
				seek = pos +1;
				state = SERVERADDR;
			break;
			case SERVERADDR:
				pos = strchr(seek, ':');
				if(pos == NULL){
					fprintf(stderr, "clientaddr_list::serveraddr: missing \':\'\n");
					return 1;
				}
				*pos = '\0';
				newelem->server->sin_family = AF_INET;
				if(pos == seek){
					inet_pton(AF_INET, "127.0.0.1", &(newelem->server->sin_addr));
				}else if(inet_pton(AF_INET, seek, &(newelem->server->sin_addr)) == 0){
					fprintf(stderr, "clientaddr_list::serveraddr: invalid ip address %s\n", seek);
					fflush(stderr);
					*pos = ':';
					return 1;
				}
				*pos = ':';
				state = SERVERPORT;
				seek = pos +1;
			break;
			case SERVERPORT:
				pos = strchr(seek, '-');
				if(pos == NULL){ // last entry
					proceed = 0;
					pos = seek + strlen(seek);
				}
				if((int)(pos-seek) < 2){
					fprintf(stderr, "clientaddr_list::serveraddr: where port?\n");
					return 1;
				}
				*pos = '\0';
				temp = strtol(seek, &endstr, 10);
				if(!(strcmp(seek, "") && *endstr == '\0')){
					fprintf(stderr, "clientaddr_list::serveraddr: bad port number; %s is not numeric\n", seek);
					return 1;
				}
				*pos = '@';
				newelem->server->sin_port = htons(temp);
				seek = pos +1;
				state = CLIENTADDR;
				ind ++;
			break;
		}
	}
	*client_list = list;
	*list_len = ind;
	return 0;
}

int	parse_input(int argc, char* argv[],
				int* listenport, 
				struct sockaddr_in* xserveraddr, 
				struct client_list** clientaddr_list, 
				int* list_len,
				int* dir)
{
	{
		int len = strlen(argv[0]) + 1;
		progname = (char*)malloc(len*sizeof(char));
		strcpy(progname, argv[0]);
	}

	int encouteredS =0;
	int encouteredU =0;
	int c;
	while((c=getopt(argc, argv, "lhs:u:")) != -1){
		switch(c){
			case 'l':
				*dir = DIR_SERVER;
			break;
			case 'h':
				print_usage(0);
				exit(0);
			break;
			case 'u':
				encouteredU =1;
				if(extract_client(optarg, clientaddr_list, list_len)){
					fprintf(stderr, "parse error, client_list is in wrong format\n");
					print_usage(1);
					return 1;
				}
			break;
			case 's':
				encouteredS =1;
				if(extract_xserver(optarg, xserveraddr)){
					fprintf(stderr, "parse error, xserver_addr is in wrong format\n");
					print_usage(1);
					return 1;
				}
			break;
			case '?':
				fprintf(stderr, "parsing error\n");
				print_usage(1);
				return 1;
			break;
		}
	}
	if(optind < argc){
		char* endstr;
		int temp = strtol(argv[optind], &endstr, 10);
		if(strcmp(argv[optind], "") && *endstr == '\0'){
			*listenport = temp;
		}else{
			fprintf(stderr, "bad port number; %s is not a numeric\n", argv[optind]);
			print_usage(1);
			return 1;
		}
	}else{
		fprintf(stderr, "listen_port_no unspecified!\n");
		print_usage(1);
		return 1;
	}
	if(!encouteredS || !encouteredU){
		fprintf(stderr, "You must provide the -s and -u flags\nsee help below\n");
		print_usage(1);
	}
	return 0;
}
