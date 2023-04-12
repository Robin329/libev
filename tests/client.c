#include <ev.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8888
#define SERVER_ADDR "127.0.0.1"

static ev_io connect_watcher;
static ev_io client_watcher;

// 定义回调函数，当服务器发送数据时被调用
static void client_cb(EV_P_ ev_io *w, int revents)
{
	char buf[1024];
	int len = read(w->fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read");
		return;
	}
	if (len == 0) {
		// 服务器已经关闭连接，停止事件循环
		ev_io_stop(EV_A_ w);
		close(w->fd);
		free(w);
		return;
	}

	printf("Received from server: %.*s\n", len, buf);
}

// 定义回调函数，当连接建立后被调用
static void connect_cb(EV_P_ ev_io *w, int revents)
{
	char buf[] = "Hello, server!";
	write(w->fd, buf, sizeof(buf));

	// 初始化ev_io对象，并注册回调函数
	ev_io_init(&client_watcher, client_cb, w->fd, EV_READ);
	ev_io_start(loop, &client_watcher);
}

int main()
{
	// 初始化libev事件循环
	struct ev_loop *loop = EV_DEFAULT;

	// 创建客户端socket，并连接到服务器
	int client_socket;
	struct sockaddr_in server_addr;
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		perror("socket");
		return 1;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_port = htons(SERVER_PORT);
	if (connect(client_socket, (struct sockaddr *)&server_addr,
		    sizeof(server_addr)) < 0) {
		perror("connect");
		return 1;
	}

	// 初始化ev_io对象，并注册回调函数
	ev_io_init(&connect_watcher, connect_cb, client_socket, EV_WRITE);
	ev_io_start(loop, &connect_watcher);

	// 开始事件循环
	ev_run(loop, 0);

	return 0;
}