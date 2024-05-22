

## 内存安全约定

1. UPtr表示拥有所有权
2. SPtr表示共享所有权
3. 裸指针表示临时对象
4. 参数传递如果不传递所有权，一般使用裸指针。
5. 使用uniqueFromInstant从临时对象转为UPtr
6. UPtr转SPtr： uptr.share();


## 内存泄漏

如果对象public继承自Refable, 那么在程序退出时控制台会打印内存泄漏情况，如果有内存泄漏需要优先处理。
