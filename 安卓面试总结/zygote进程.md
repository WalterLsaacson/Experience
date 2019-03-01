#### Zygote进程启动及相关操作

* zygote进程以服务的方式通过rc文件的触发器机制触发并启动 root/init.zygote64_32.rc

```makefile
service zygote /system/bin/app_process64 -Xzygote /system/bin --zygote --start-system-server --socket-name=zygote
    class main //类别定义为main 对应的触发器为on nonencrypted
    class_start main
    priority -20
    user root
    group root readproc reserved_disk
    socket zygote stream 660 root system
    onrestart write /sys/android_power/request_state wake
    onrestart write /sys/power/state on
    onrestart restart audioserver
    onrestart restart cameraserver
    onrestart restart media
    onrestart restart netd
    onrestart restart wificond
    writepid /dev/cpuset/foreground/tasks
```

