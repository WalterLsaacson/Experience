##### dumpsys命令是什么：

先来看一下官方对dumpsys的描述：

> `dumpsys` is a tool that runs on Android devices and provides information about system services.
>
> We can use `dumpsys` to accomplish common tasks, such as inspecting input, RAM, battery, or network diagnostics.

可以看到dumpsys是android系统提供的和系统服务打交道的工具，甚至可以直接改变系统服务的某些状态值，官方列举了几个常见的任务：检查输入，RAM，电池或网络诊断。

##### dumpsys命令怎么用：

> adb shell dumpsys --help
>
> usage: dumpsys
> ​         To dump all services.
> or:
> ​       dumpsys [-t TIMEOUT][--help | -l | --skip SERVICES | SERVICE [ARGS]]
> ​         --help: shows this help
> ​         -l: only list services, do not dump them
> ​         -t TIMEOUT: TIMEOUT to use in seconds instead of default 10 seconds
> ​         --skip SERVICES: dumps all services but SERVICES (comma-separated list)
> ​         SERVICE [ARGS]: dumps only service SERVICE, optionally passing ARGS to it

* 直接使用`adb shell dumpsys`命令系统将把所有系统服务的状态打印出来，适用于在系统发生了异常时，获取系统当前状态的 一个途径

* `adb shell dumpsys -l`：将所有可以dump的系统服务列举出来

* `adb shell dumpsys -t`:后边跟一个数字以秒为单位，表示将在多少秒之后进行dump操作

* `adb shell dumpsys --skip SERVICES`在dumpsys的过程中把某些系统服务过滤掉

* `adb shell dumpsys SERVICE[ARGS]`指定某一个系统服务进行dump

  a.根据不同的系统服务加不同的参数,通过`dumpsys SERVICE help`查看相关的参数说明

  b.与-t参数配合取得指定延迟之后的某条服务状态信息

##### 举例：

* 获取当前的前台activity

`adb shell dumpsys activity activities | grep mFocusedActivity`


* 获取当前电池状态

`adb shell dumpsys battery`

* 获取设置应用的内存信息

`adb shell dumpsys meminfo com.android.settings`