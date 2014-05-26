push
====

百万连接测试

对于推送技术，一般需要和客户端建立一个长连接，所以需要测试一下单台机器能够建立的链接数据以及推送能力。

此项目用于做一些基准测试以及性能测试程序。

现在的程序主要是用于创建tcp长连接的程序。


主要的方法，是在单台主机上建立20个左右的虚拟ip地址，每个虚拟ip地址大约有6w个可用的端口号，差不多可以
和服务器端建立20*6w ~ 100w个tcp长连接。

1.ipup.sh和ipdown.sh两个脚本用来开启和关闭虚拟ip地址
2.server现在主要是监听客户端的连接，client端通过虚拟ip地址访问server端口。



统计数据：

socket建立连接数：

![socket建立连接数](https://raw.githubusercontent.com/fisheuler/push/master/image/tcp-socket.jpg)



可以参考的文档连接地址：

1.
A Million-user Comet Application with Mochiweb

http://www.metabrew.com/article/a-million-user-comet-application-with-mochiweb-part-3

2.

100万并发连接服务器笔记之处理端口数量受限问题

http://www.blogjava.net/yongboy/archive/2013/04/09/397594.html

