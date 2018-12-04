1. 属性触发器

```makefile
on property:sys.usb.configfs=1
    start adbd
```

表示一旦属性sys.usb.configfs的值被设置为1，将执行之后的语句，这里是启动adbd进程