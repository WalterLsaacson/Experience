## init进程

### 加载方式

源码目录：/system/core/init

* 模块编译文件中指定该模块将编译为一个二进制文件

```makefile
Android.mk//节选
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(init_cflags)
LOCAL_SRC_FILES := main.cpp

LOCAL_MODULE:= init

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)//编译生成文件在/root/下
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

include $(BUILD_EXECUTABLE)//指定编译为可执行文件
```

引入新的Android BP文件之后，在BP文件中也添加了相关的代码，不过还没有得到实际使用

```makefile
/*
This is not yet ready, see the below TODOs for what is missing

cc_binary {//指定编译为二进制可执行文件
    // TODO: Missing,
    //LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
    //LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

    name: "init",
    defaults: ["init_defaults"],
    required: [
        "e2fsdroid",
        "mke2fs",
        "sload_f2fs",
        "make_f2fs",
    ],
    static_executable: true,
    srcs: ["main.cpp"],
    symlinks: [
        "sbin/ueventd",
        "sbin/watchdogd",
    ],
}
*/
```

* 编译生成的二进制文件init将在kernel启动完成后由kernel引导执行

### 执行的操作

* 编译模块中指定的源码文件为main.cpp，这里只是作为一个代理，跳转到真正启动流程的init.cpp中

```c++
#include "init.h"

int main(int argc, char** argv) {
    android::init::main(argc, argv);//跳转执行
}
```

* 关注一下该进程执行的一些重要操作

* 将相关的二进制文件目录添加进系统的环境变量中

  ```cpp
  setenv("PATH", _PATH_DEFPATH, 1);
  //导入相关头文件#include <paths.h>该变量在此头文件中定义
  //_PATH_DEFPATH	//"/sbin:/system/sbin:/system/bin:/system/xbin:/odm/bin:/vendor/bin:/vendor/xbin"
  ```

* 执行和文件系统相关的操作

```cpp
mount//挂载文件节点
    mount("tmpfs", "/dev", "tmpfs", MS_NOSUID, "mode=0755");
mkdir//创建文件夹
    mkdir("/dev/pts", 0755);
    mkdir("/dev/socket", 0755);
mksnod//创建文件节点
    mknod("/dev/kmsg", S_IFCHR | 0600, makedev(1, 11));
```

* selinux初始化

```cpp
SelinuxInitialize()
```

* 初始化系统属性服务

```cpp
property_init();
mkdir("/dev/__properties__", S_IRWXU | S_IXGRP | S_IXOTH);//创建存放属性的文件
```

* 加载来自kernel的系统属性

```cpp
process_kernel_cmdline();//可以通过此方法在kernel中设置系统属性
通过adb shell cat /proc/cmdline可以获取保存在节点的该变量值
```

* 加载来自编译系统的各种属性

```cpp
property_load_boot_defaults();//指定相关需要加载的文件名然后进行解析
```

* 创建属性服务的socket服务端

```cpp
start_property_service()
    property_set_fd = CreateSocket(PROP_SERVICE_NAME, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,false, 0666, 0, 0, nullptr);
```

* 创建USB相关

```cpp
set_usb_controller
```

* 通过两个例子描述一下rc文件的语法

  ```makefile
  1.//定义服务，可以通过adb shell ctrl start servicename 的方式启动
  service bootanim /system/bin/bootanimation//service 服务名称 启动该服务将执行的bin文件
      class core animation//系统核心服务，包含在animation类别中，将通过触发器和该类别其他服务一起启动
      user graphics
      group graphics audio
      disabled//默认不启动，必须显式启动
      oneshot//服务退出后不会重启
      writepid /dev/stune/top-app/tasks
      
  2.定义action
  on charger//表明将在charger触发时执行，一般这里的触发器通过属性值控制。
  			例如：on property:vold.decrypt=trigger_load_persist_props
      class_start charger//启动通过定义服务的方式定义的服务
  ```

  

这篇文章有比较详细的解析[https://www.jianshu.com/p/d08e1affd5ec]

系统源码中也有相关的文档:system/cor/init/README.md

* 加载rc文件机制定义的各种服务，通过不同的action触发相关需要执行的服务

  * 获取相关数据结构的单例
  ```cpp
  ActionManager& am = ActionManager::GetInstance();//保存定义好的action并通过多路复用的方式触发
  ServiceList& sm = ServiceList::GetInstance();//将定义的service以列表形式保存
  ```
  * 解析指定目录的rc文件

    ```cpp
    static void LoadBootScripts(ActionManager& action_manager, ServiceList& service_list) {
        Parser parser = CreateParser(action_manager, service_list);
        std::string bootscript = GetProperty("ro.boot.init_rc", "");
        if (bootscript.empty()) {
            parser.ParseConfig("/init.rc");
            if (!parser.ParseConfig("/system/etc/init")) {
                late_import_paths.emplace_back("/system/etc/init");
            }
            if (!parser.ParseConfig("/product/etc/init")) {
                late_import_paths.emplace_back("/product/etc/init");
            }
            if (!parser.ParseConfig("/odm/etc/init")) {
                late_import_paths.emplace_back("/odm/etc/init");
            }
            if (!parser.ParseConfig("/vendor/etc/init")) {
                late_import_paths.emplace_back("/vendor/etc/init");
            }
        } else {
            parser.ParseConfig(bootscript);
        }
    }
    ```

  有些服务或者action没有直接在上述这几个文件中定义，他们定义在自己的目录下的rc中，通过rc文件的import语法导入并在这里加载

  ```makefile
  import /init.environ.rc
  import /init.usb.rc
  //导入root文件夹下的两个rc文件
  ```

  * 触发系统初始化的几个action

  ```cpp
  am.QueueEventTrigger("early-init");
  
      // Queue an action that waits for coldboot done so we know ueventd has set up all of /dev...
      am.QueueBuiltinAction(wait_for_coldboot_done_action, "wait_for_coldboot_done");
      // ... so that we can start queuing up actions that require stuff from /dev.
      am.QueueBuiltinAction(MixHwrngIntoLinuxRngAction, "MixHwrngIntoLinuxRng");
      am.QueueBuiltinAction(SetMmapRndBitsAction, "SetMmapRndBits");
      am.QueueBuiltinAction(SetKptrRestrictAction, "SetKptrRestrict");
      am.QueueBuiltinAction(keychord_init_action, "keychord_init");
      am.QueueBuiltinAction(console_init_action, "console_init");
  
      // Trigger all the boot actions to get us started.
      am.QueueEventTrigger("init");
  
      // Repeat mix_hwrng_into_linux_rng in case /dev/hw_random or /dev/random
      // wasn't ready immediately after wait_for_coldboot_done
      am.QueueBuiltinAction(MixHwrngIntoLinuxRngAction, "MixHwrngIntoLinuxRng");
  
      // Don't mount filesystems or start core system services in charger mode.
      std::string bootmode = GetProperty("ro.bootmode", "");
      if (bootmode == "charger") {
          am.QueueEventTrigger("charger");
      } else {
          am.QueueEventTrigger("late-init");
      }
  
      // Run all property triggers based on current state of the properties.
      am.QueueBuiltinAction(queue_property_triggers_action, "queue_property_triggers");
  ```

  

  * 进入死循环等待触发器触发

  ```CPP
  while (true) {
          // By default, sleep until something happens.
          int epoll_timeout_ms = -1;
  
          if (do_shutdown && !shutting_down) {
              do_shutdown = false;
              if (HandlePowerctlMessage(shutdown_command)) {
                  shutting_down = true;
              }
          }
  
          if (!(waiting_for_prop || Service::is_exec_service_running())) {
              am.ExecuteOneCommand();
          }
          if (!(waiting_for_prop || Service::is_exec_service_running())) {
              if (!shutting_down) {
                  auto next_process_restart_time = RestartProcesses();
  
                  // If there's a process that needs restarting, wake up in time for that.
                  if (next_process_restart_time) {
                      epoll_timeout_ms = std::chrono::ceil<std::chrono::milliseconds>(
                                             *next_process_restart_time - boot_clock::now())
                                             .count();
                      if (epoll_timeout_ms < 0) epoll_timeout_ms = 0;
                  }
              }
  
              // If there's more work to do, wake up again immediately.
              if (am.HasMoreCommands()) epoll_timeout_ms = 0;
          }
  
          epoll_event ev;
          int nr = TEMP_FAILURE_RETRY(epoll_wait(epoll_fd, &ev, 1, epoll_timeout_ms));
          if (nr == -1) {
              PLOG(ERROR) << "epoll_wait failed";
          } else if (nr == 1) {
              ((void (*)()) ev.data.ptr)();
          }
      }
  ```

  这里通过epoll机制进行等待和触发

  ### 储备知识

  * 阻塞IO，非阻塞IO多路复用
  * 本地socket连接通信

   