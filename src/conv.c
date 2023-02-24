#include "conv.h"

#define FBUFFSIZ 8192

#define MIN(x,y) ((x) > (y) ? (y) : (x))

int create_http_request(char* srcbuff, int len, char* destbuff, struct sockaddr_in* addr){
	char ipstr[17];
	inet_ntop(AF_INET, &(addr->sin_addr), ipstr, 17);
	int port = ntohs(addr->sin_port);

	time_t timev;
	time(&timev);
	char timebuff[64];
	strcpy(timebuff, ctime(&timev));
	timebuff[strlen(timebuff)-1] = '\0';
	sprintf(destbuff,	"HTTP/2 200 OK\r\n"
						"Server: nginx\r\n"
						"Host: t2.ex.bruhmoment.ru\r\n"
						"date: %s\r\n"
						"content-type: image/png\r\n"
						"content-length: %d\r\n"
						"content-disposition: inline; filename=\"th-%d\"\r\n"
						"x-frame-options: SAMEORIGIN\r\n"
						"x-xss-protection: 1;mode=block\r\n"
						"x-content-type-options: nosniff\r\n"
						"D-senderip: %s\r\n"
						"D-senderport: %d\r\n"
						"\r\n"
						"%s", timebuff, len, rand()%10000000, ipstr, port, srcbuff);
	
	return strlen(destbuff);
}

int extractline(char* linebuff, char* pool, int curr_index, int last_index){
	int cont = 1;
	int off = 0;
	while(cont && curr_index < last_index){
		if(pool[curr_index] == '\n') cont = 0;
		linebuff[off++] = pool[curr_index ++];
	}
	linebuff[off] = '\0';
	return off;
}

char floating_buff[FBUFFSIZ];
int curr_index = 0;
int last_index = 0;

int decode_one_udp(struct queue* queue, char* outbuff, struct sockaddr_in* addr){
	enum {
		S_HTTP,
		S_LENGTH,
		S_IP,
		S_PORT,
		S_CONT,
		S_READ
	} state;
	state = S_HTTP;
	struct queue_node* temp = NULL;
	if(curr_index != last_index){
		memcpy(floating_buff, floating_buff + curr_index, last_index - curr_index);
		last_index -= curr_index;
		curr_index = 0;
	}
	char linebuff[512];
	int linelen;

	int contentlength;	
	char ipstr[17];
	int portno;

	int read_offset =0;

	while(1){
		if(curr_index == last_index && (state != S_READ || read_offset != contentlength)){
			if(temp) free(temp->buffer);
			free(temp);
			temp = queue_pop(queue);
			memcpy(floating_buff + curr_index, temp->buffer, temp->datasz);
			last_index = curr_index + temp->datasz;
		}
		if(state == S_READ){
			if(read_offset == contentlength) {
				outbuff[read_offset] = '\0';
				break;
			}
			outbuff[read_offset++] = floating_buff[curr_index ++];
			continue;
		}
		linelen = extractline(linebuff, floating_buff, curr_index, last_index);
		switch(state){
			case S_HTTP:
				if(!strncmp(linebuff, "HTTP/2 200 OK\r\n", MIN(linelen, 15))){
					state = S_LENGTH;
				}
			break;
			case S_LENGTH:
				if(!strncmp(linebuff, "content-length: ", MIN(linelen, 16))){
					sscanf(linebuff, "content-length: %d\r\n", &contentlength);
					state = S_IP;
				}
			break;
			case S_IP:
				if(!strncmp(linebuff, "D-senderip: ", MIN(linelen, 12))){
					sscanf(linebuff, "D-senderip: %s\r\n", ipstr);
					state = S_PORT;
				}
			break;
			case S_PORT:
				if(!strncmp(linebuff, "D-senderport: ", MIN(linelen, 14))){
					sscanf(linebuff, "D-senderport: %d\r\n", &portno);
					state = S_CONT;
				}
			break;
			case S_CONT:
				if(linebuff[0] == '\r' && linebuff[1] == '\n'){
					state = S_READ;
				}
			break;
		}
		if(curr_index + linelen < last_index) curr_index += linelen; 
		else curr_index = last_index;
	}
	if(temp) free(temp->buffer);
	free(temp);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(portno);
	inet_pton(AF_INET, ipstr, &(addr->sin_addr));
	return read_offset;
}

void* conv_proc(void* vargs){
	srand(time(NULL));
	struct thread_arg* targ = (struct thread_arg*)vargs;
	struct queue* loutqueue = targ->queue_list + 1;
	struct queue* linqueue = targ->queue_list + 0;
	struct queue* routqueue = targ->queue_list + 2;
	struct queue* rinqueue = targ->queue_list + 3;

	int lincnt =0;
	int rincnt =0;

	while(!_exitsig){
		lincnt = linqueue->len;
		rincnt = rinqueue->len;
		if(!(lincnt || rincnt)){
			sleep(0.3);
			continue;
		}
		if(lincnt){
			struct queue_node* nn = (struct queue_node*)malloc(sizeof(struct queue_node));
			nn->buffer = (char*)malloc(TCPBUFSIZ);
			struct queue_node* ln = queue_pop(linqueue);
			struct sockaddr_in* destaddr = targ->client_list[ln->index].server;
			
			int packlen = create_http_request(ln->buffer, ln->datasz, nn->buffer, destaddr);

			nn->datasz = packlen;

			queue_push(routqueue, nn);
			free(ln->buffer);
			free(ln);
		}
		if(rincnt){
			struct queue_node* nn = (struct queue_node*)malloc(sizeof(struct queue_node));
			nn->buffer = (char*)malloc(UDPBUFSIZ);
			struct sockaddr_in temp;

			int packlen = decode_one_udp(rinqueue, nn->buffer, &temp);
			
			printf("converter yielded[%d]: %s\n", packlen, nn->buffer);
			fflush(stdout);
			nn->datasz = packlen;

			nn->index = search_list(targ->client_list, targ->list_len, &temp, SEL_CLIENT);
			if(index < 0){
				eprint("address not found!\ndropping\n");
				free(nn->buffer);
				free(nn);
				continue;
			}

			queue_push(loutqueue, nn);
		}
	}
	return NULL;
}
