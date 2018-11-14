## 添加adb shell dumpsys recovery reset-factory

- 将添加adb shell dumpsys recovery reset-factory 命令直接将手机恢复出厂设置
- 通过之前的分析可以看到整个dump的框架已经存在了，我们接下来只需要在特定的service中重写dump方法，并实现功能即可

1. 查询recovery对应的系统服务

通过第二节的描述，直接在Context类中找到recovery字段的定义

```java
public static final String RECOVERY_SERVICE = "recovery";
```

接下来在SystemServiceRegistry中寻找对应的service

```java
registerService(Context.RECOVERY_SERVICE, RecoverySystem.class,
                //service的封装类
        new CachedServiceFetcher<RecoverySystem>() {
    @Override
    public RecoverySystem createService(ContextImpl ctx) throws ServiceNotFoundException {
        IBinder b = ServiceManager.getServiceOrThrow(Context.RECOVERY_SERVICE);
        //继承自如下类的service将是我们需要寻找的service
        //IRecoverySystem.Stub
        IRecoverySystem service = IRecoverySystem.Stub.asInterface(b);
        return new RecoverySystem(service);
    }});
```

最终定位到RecoverySystemService类中，接下来尝试添加指令方法.

2. 添加相应的dump方法实现

```java
   /**
     * 响应recovery reset-factory指令，执行重置操作
     *
     * @param fd 管道通信的上层输出端
     * @param pw 创建的文件输出流，直接调用print方法写文件
     * @param args 命令行参数数组，对应recovery之后的部分，以空白描述符为分隔符
     */
   @Override
   protected void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        if (args!=null&&args.length>0) {
            //取参数，参数错误时输出帮助信息
            if (args[0].equals("reset-factory")) {
                //输出信息表示正在执行操作
                pw.println("Reset to factory");
                try{
                    //RecoverySystem.rebootWipeUserData(mContext, false, "reset-factory", true, false);
                    synchronized (sRequestLock) {
                        String command = "--wipe_data\n--reason=reset-factory\n--locale=zh-CN";
                        if (!setupOrClearBcbInternal(true, command)) {
                            return;
                        }
                        Runnable runnable = new Runnable() {
                            @Override
                            public void run() {
                                ShutdownThread.reboot(getUiContext(), PowerManager.REBOOT_RECOVERY, false);
                            }
                        };

                        // ShutdownThread must run on a looper capable of displaying the UI.
                        Message msg = Message.obtain(UiThread.getHandler(), runnable);
                        msg.setAsynchronous(true);
                        UiThread.getHandler().sendMessage(msg);
                    }
                }catch(Exception e){
                    Slog.e(TAG, "rebootWipeUserData failed", e);
                    pw.println("reset failed,"+e.getMessage());
                }
                return;
            } else {
                // --help
                pw.pringln("Usage: 'dumpsys recovery reset-factory'");
                pw.pringln("Restore the phone to factory Settings");
            }
        }
    }
```

