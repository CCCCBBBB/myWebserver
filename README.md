# C++ Webserver library
## 项目介绍
使用c++11并参考muduo实现的基于多Reactor模型的高并发TCP网络库，其中包含异步日志模块，用于压测的简易HTTPserver模块等。
## 项目特点
* 基于主从的多Reactor模型，使用`Epoll + LT`模型的I/O复用模型
* 参考muduo中的`one loop per thread`思想，线程池轮询工作
* 使用`eventfd`作为线程异步唤醒手段
* 遵循`RAII`原则，使用智能指针进行多线程的内存管理
* 实现基于`双缓存异步日志`模块，前端缓存日志信息，后端线程负责定期的交换缓存及数据落盘
## 多Reactor模型
