#include <ev.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8888

// 客户端连接结构体
typedef struct {
	struct ev_io io;
	int socket;
} client_t;

static ev_io accept_watcher;

// 定义回调函数，当有客户端发送数据时被调用
static void client_cb(EV_P_ ev_io *w, int revents)
{
	char buf[1024];
	int len = read(w->fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read");
		return;
	}
	if (len == 0) {
		// 客户端已经关闭连接，停止事件循环
		ev_io_stop(EV_A_ w);
		close(w->fd);
		free(w);
		return;
	}

	// 将接收到的数据原样发送回客户端
	write(w->fd, buf, len);
}

// 定义回调函数，当有客户端连接时被调用
static void accept_cb(EV_P_ ev_io *w, int revents)
{
	int client_socket;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// 接受客户端连接请求
	client_socket =
		accept(w->fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_socket < 0) {
		perror("accept");
		return;
	}

	// 将客户端连接信息保存到client_t结构体中
	client_t *client = (client_t *)malloc(sizeof(client_t));
	if (client == NULL) {
		perror("malloc");
		close(client_socket);
		return;
	}
	memset(client, 0, sizeof(client_t));
	client->socket = client_socket;

	// 初始化ev_io对象，并注册回调函数
	ev_io_init(&client->io, client_cb, client_socket, EV_READ);
	ev_io_start(loop, &client->io);

	printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr),
	       ntohs(client_addr.sin_port));
}

int main()
{
	// 初始化libev事件循环
	struct ev_loop *loop = EV_DEFAULT;

	// 创建服务器socket，并绑定端口
	int server_socket;
	struct sockaddr_in server_addr;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("socket");
		return 1;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);
	if (bind(server_socket, (struct sockaddr *)&server_addr,
		 sizeof(server_addr)) < 0) {
		perror("bind");
		return 1;
	}

	// 开始监听端口
	if (listen(server_socket, SOMAXCONN) < 0) {
		perror("listen");
		return 1;
	}

	// 初始化ev_io对象，并注册回调函数
	ev_io_init(&accept_watcher, accept_cb, server_socket, EV_READ);
	ev_io_start(loop, &accept_watcher);

	printf("Server listening on port %d\n", SERVER_PORT);

	// 开始事件循环
	ev_run(loop, 0);

	return 0;
}