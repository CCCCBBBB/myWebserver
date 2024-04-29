# C++ Webserver library
## 项目介绍
使用c++11并参考muduo实现的基于多Reactor模型的高并发TCP网络库，其中包含异步日志模块，用于压测的简易HTTPserver模块等。
## 项目特点
* 基于主从的多Reactor模型，使用`Epoll + LT`模型的I/O复用模型
* 参考muduo中的`one loop per thread`思想，线程池轮询工作
* 使用`eventfd`作为线程异步唤醒手段
* 遵循`RAII`原则，使用智能指针进行多线程的内存管理
* 实现基于`双缓存异步日志`模块，前端缓存日志信息，后端线程负责定期的交换缓存及数据落盘
* 封装性好，用户只用关心业务逻辑回调处理函数即可
## 多Reactor模型
![](https://github.com/CCCCBBBB/myWebserver/blob/main/images/%E5%A4%9AReactor.png)
MainReactor通过Acceptor监听并接受新连接，并将其轮询派发到subReactor，subReactor就负责监听处理这个连接的读写事件

## 项目构建
```
./build.sh
```
运行示例 默认访问127.0.0.1：8000/
```
./webserver
```
## 压测
使用webbench1.5 进行测试 如下图
![](https://github.com/CCCCBBBB/myWebserver/blob/main/images/%E5%8E%8B%E6%B5%8B.png)
