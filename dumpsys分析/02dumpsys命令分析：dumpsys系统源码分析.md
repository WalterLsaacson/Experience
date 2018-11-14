## adb dumpsys源码分析

源码位置：`platform\frameworks\native\cmds\dumpsys`

#### 通过adb shell执行dumpsys工具

- dumpsys模块使用Android.bp文件进行编译配置，指定cc_binary字段表示将编译该模块为一个二进制可执行文件，因此我们可以使用adb shell dumpsys命令来执行该模块的代码

配置文件摘要如下：

```bp
//
// Executable
//

cc_binary {
    name: "dumpsys",

    defaults: ["dumpsys_defaults"],

    srcs: [
        "main.cpp",
    ],
}
```

srcs参数表示指定的第一个文件是main.cpp文件

#### dumpsys -l 是怎样列举的

通过dumpsys -l 罗列出来的名称是与framework层的系统服务一一对应的

```
Such as:
activity ActivityManagerService
window   WindowManagerSewrvice
```

这些对应关系是在framework的SystemServiceRegistry类中进行注册的：

```java
 //注册ActivityManagerService服务，将保存在一个map中，键为Context中定义的字段，值为相应的系统服务
 registerService(Context.ACTIVITY_SERVICE, ActivityManager.class,
                new CachedServiceFetcher<ActivityManager>() {
            ...
            }});

 registerService(Context.ALARM_SERVICE, AlarmManager.class,
                new CachedServiceFetcher<AlarmManager>() {
            ...
            }});
```

#### 可执行文件与系统服务打交道

- dumpsys工具的流程就是调用ServiceManager服务的listServices查询系统注册的所有服务，然后嗲用checkservice接口获取服务的远程binder代理对象，最终调用服务的dump函数并传递相应的参数用来打印该服务的特定信息
- 过程涉及进程间的binder通信，pipe管道通信
- 我们来看一下代码

1.   main.cpp

```c++
//argc参数数量，argv参数数组索引0表示参数名
int main(int argc, char* const argv[]) {
    //adb通过tcp协议与手机adbd进程进行通信，这里防止手机端产生僵尸进程
    signal(SIGPIPE, SIG_IGN);
    //获取IServiceManager对象
    sp<IServiceManager> sm = defaultServiceManager();
    //清空输出
    fflush(stdout);
    if (sm == nullptr) {
        ALOGE("Unable to get default service manager!");
        aerr << "dumpsys: Unable to get default service manager!" << endl;
        return 20;
    }

    Dumpsys dumpsys(sm.get());
    //调用dumpsys.cpp文件的main函数
    return dumpsys.main(argc, argv);
}
```

2. dumpsys.cpp

```c++
int Dumpsys::main(int argc, char* const argv[]) {
    //定义局部变量将参与到后续的显示工作中
    Vector<String16> services;
    Vector<String16> args;
    String16 priorityType;
    Vector<String16> skippedServices;
    Vector<String16> protoServices;
    //参与解析工作的布尔控制变量,名称含义比较明确
    bool showListOnly = false;
    bool skipServices = false;
    bool asProto = false;
    int timeoutArgMs = 10000;
    //最终将参与StringAppendF函数的打印中
    int priorityFlags = IServiceManager::DUMP_FLAG_PRIORITY_ALL;
    static struct option longOptions[] = {{"priority", required_argument, 0, 0},
                                          {"proto", no_argument, 0, 0},
                                          {"skip", no_argument, 0, 0},
                                          {"help", no_argument, 0, 0},
                                          {0, 0, 0, 0}};

    // Must reset optind, otherwise subsequent calls will fail (wouldn't happen on main.cpp, but
    // happens on test cases).
    optind = 1;
    while (1) {
        int c;
        int optionIndex = 0;
        
        //通过getopt_long循环获取长参数列表中的参数，并一一解析
        c = getopt_long(argc, argv, "+t:T:l", longOptions, &optionIndex);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            if (!strcmp(longOptions[optionIndex].name, "skip")) {
                skipServices = true;
            } else if (!strcmp(longOptions[optionIndex].name, "proto")) {
                asProto = true;
            } else if (!strcmp(longOptions[optionIndex].name, "help")) {
                usage();
                return 0;
            } else if (!strcmp(longOptions[optionIndex].name, "priority")) {
                priorityType = String16(String8(optarg));
                if (!ConvertPriorityTypeToBitmask(priorityType, priorityFlags)) {
                    fprintf(stderr, "\n");
                    usage();
                    return -1;
                }
            }
            break;

        case 't':
            {
                char* endptr;
                timeoutArgMs = strtol(optarg, &endptr, 10);
                timeoutArgMs = timeoutArgMs * 1000;
                if (*endptr != '\0' || timeoutArgMs <= 0) {
                    fprintf(stderr, "Error: invalid timeout(seconds) number: '%s'\n", optarg);
                    return -1;
                }
            }
            break;

        case 'T':
            {
                char* endptr;
                timeoutArgMs = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || timeoutArgMs <= 0) {
                    fprintf(stderr, "Error: invalid timeout(milliseconds) number: '%s'\n", optarg);
                    return -1;
                }
            }
            break;

        case 'l':
            showListOnly = true;
            break;

        default:
            fprintf(stderr, "\n");
            usage();
            return -1;
        }
    }

    for (int i = optind; i < argc; i++) {
        if (skipServices) {、
            //在第一篇文件中有提到可以直接使用dumpsys工具并指定export的服务，这里添加要export的服务
            skippedServices.add(String16(argv[i]));
        } else {
            if (i == optind) {
                services.add(String16(argv[i]));
            } else {
                args.add(String16(argv[i]));
            }
        }
    }

    //跳过服务没有指定跳过哪些服务
    if ((skipServices && skippedServices.empty()) ||
            (showListOnly && (!services.empty() || !skippedServices.empty()))) {
        usage();
        return -1;
    }

    //只展示服务列表
    if (services.empty() || showListOnly) {
        services = listServices(priorityFlags, asProto);
        setServiceArgs(args, asProto, priorityFlags);
    }

    const size_t N = services.size();
    if (N > 1) {
        // first print a list of the current services
        aout << "Currently running services:" << endl;

        for (size_t i=0; i<N; i++) {
            sp<IBinder> service = sm_->checkService(services[i]);

            if (service != nullptr) {
                bool skipped = IsSkipped(skippedServices, services[i]);
                aout << "  " << services[i] << (skipped ? " (skipped)" : "") << endl;
            }
        }
    }

    if (showListOnly) {
        return 0;
    }

    for (size_t i = 0; i < N; i++) {
        const String16& serviceName = services[i];
        if (IsSkipped(skippedServices, serviceName)) continue;

        //新开一个线程进行服务的dump操作
        if (startDumpThread(serviceName, args) == OK) {
            bool addSeparator = (N > 1);
            if (addSeparator) {
                writeDumpHeader(STDOUT_FILENO, serviceName, priorityFlags);
            }
            std::chrono::duration<double> elapsedDuration;
            size_t bytesWritten = 0;
            status_t status =
            //读取结果
                writeDump(STDOUT_FILENO, serviceName, std::chrono::milliseconds(timeoutArgMs),
                          asProto, elapsedDuration, bytesWritten);

            if (status == TIMED_OUT) {
                aout << endl
                     << "*** SERVICE '" << serviceName << "' DUMP TIMEOUT (" << timeoutArgMs
                     << "ms) EXPIRED ***" << endl
                     << endl;
            }
            //
            if (addSeparator) {
                writeDumpFooter(STDOUT_FILENO, serviceName, elapsedDuration);
            }
            bool dumpComplete = (status == OK);
            //停止dump操作
            stopDumpThread(dumpComplete);
        }
    }

    return 0;
}
```

3. 开线程dumpsys

```c++
status_t Dumpsys::startDumpThread(const String16& serviceName, const Vector<String16>& args) {
    //1. 调用servicemanager的checkservice方法获取相应service的binder代理对象
    sp<IBinder> service = sm_->checkService(serviceName);
    if (service == nullptr) {
        aerr << "Can't find service: " << serviceName << endl;
        return NAME_NOT_FOUND;
    }

    int sfd[2];
    //2. 系统调用创建一个简单的管道，sfd[0]用于读取管道，sfd[1]用于写入管道
    if (pipe(sfd) != 0) {
        aerr << "Failed to create pipe to dump service info for " << serviceName << ": "
             << strerror(errno) << endl;
        return -errno;
    }

    //3. 管道通信的输入和输出均使用fd进行包装
    redirectFd_ = unique_fd(sfd[0]);
    unique_fd remote_end(sfd[1]);
    sfd[0] = sfd[1] = -1;

    // dump blocks until completion, so spawn a thread..
    activeThread_ = std::thread([=, remote_end{std::move(remote_end)}]() mutable {
    //4. 调用service的dump方法，上层通过进程binder类实现该功能
        int err = service->dump(remote_end.get(), args);

        // It'd be nice to be able to close the remote end of the socketpair before the dump call returns, to terminate our reads if the other end closes their copy of the file descriptor, but then hangs for some reason. There doesn't seem to be a good way to do this, though.
        remote_end.reset();

        if (err != 0) {
            aerr << "Error dumping service info: (" << strerror(err) << ") "
                 << serviceName << endl;
        }
    });
    return OK;
}

status_t Dumpsys::writeDump(int fd, const String16& serviceName, std::chrono::milliseconds timeout,
                            bool asProto, std::chrono::duration<double>& elapsedDuration,
                            size_t& bytesWritten) const {
    //...
    //5. 系统调用poll读取管道数据
    struct pollfd pfd = {.fd = serviceDumpFd, .events = POLLIN};
    while (true) {
        // Wrap this in a lambda so that TEMP_FAILURE_RETRY recalculates the timeout.
        auto time_left_ms = [end]() {
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - now);
            return std::max(diff.count(), 0ll);
        };
        int rc = TEMP_FAILURE_RETRY(poll(&pfd, 1, time_left_ms()));
        if (rc < 0) {
            aerr << "Error in poll while dumping service " << serviceName << " : "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        } else if (rc == 0) {
            status = TIMED_OUT;
            break;
        }

        char buf[4096];
        //6. 读取管道数据，TEMP_FAILURE_RETRY宏将在失败后重试
        rc = TEMP_FAILURE_RETRY(read(redirectFd_.get(), buf, sizeof(buf)));
        if (rc < 0) {
            aerr << "Failed to read while dumping service " << serviceName << ": "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        } else if (rc == 0) {
            // EOF.
            break;
        }
        //7. 写数据到fd
        if (!WriteFully(fd, buf, rc)) {
            aerr << "Failed to write while dumping service " << serviceName << ": "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        }
        totalBytes += rc;
    }
    return status;
}
```

4. 过程中的binder调用

这一块抽出去在后边的文章专门进行讨论。

4. framework层系统服务

系统服务通过继承binder类并重写其dump方法，完成最终获取framework信息的步骤

```java
@Override
public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
    PriorityDump.dump(mPriorityDumper, fd, pw, args);
}

//最终调用
// Wrapper function to print out debug data filtered by specified arguments.
    private void doDump(FileDescriptor fd, PrintWriter pw, String[] args, boolean useProto)
```