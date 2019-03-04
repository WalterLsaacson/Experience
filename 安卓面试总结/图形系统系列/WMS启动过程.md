#### 启动过程

* SystemServer --->startOtherServices()
* 在android.display进程中执行WMS的构造方法
  * 使用**handler**的**runWithScissors()**方法
  * 此方法是一个同步方法，当前进程会阻塞，到方法执行完返回之后，才返回之前的进程继续执行操作

* WMS的构造方法创建DisplayContent对象，用于支持多屏显示

* 初始化策略类initPolicy方法中获取UI线程的handler进行相关的 初始化操作。这里同样是使用handler

  的**runWithScissors()**方法，只不过这次阻塞的是android.display线程，执行操作的是android.ui线程

* displayReady之后进行WMS自身和PhoneWindowManager的displayReady调用，进行相关的初始化操作

#### 启动窗口(StartingWindow)