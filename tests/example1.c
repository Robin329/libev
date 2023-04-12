
#include <ev.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

// 定义回调函数，当stdin上有数据时会被调用
static void stdin_cb(EV_P_ ev_io *w, int revents)
{
	char buf[1024];
	int len = read(w->fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read");
		return;
	}
	if (len == 0) {
		// EOF，停止事件循环
		ev_io_stop(EV_A_ w);
		return;
	}
	printf("Read %d bytes from stdin: %.*s", len, len, buf);
}

int main()
{
	// 初始化libev事件循环
	struct ev_loop *loop = EV_DEFAULT;

	// 创建ev_io对象，并注册回调函数
	ev_io stdin_watcher;
	ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
	ev_io_start(loop, &stdin_watcher);

	// 开始事件循环
	ev_run(loop, 0);

	return 0;
}