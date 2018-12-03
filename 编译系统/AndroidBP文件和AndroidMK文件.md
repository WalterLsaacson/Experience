#### BP文件解析（待完善）

Android BP文件是用来替换Android MK文件的配置文件，将被Blueprint框架解析为Ninja文件。

```
Android.bp --> Blueprint --> Soong --> Ninja
Makefile or Android.mk --> kati --> Ninja
(Android.mk --> Soong --> Blueprint --> Android.bp)
```

#### 两种文件相互转换

安卓使用bp文件替代mk文件之后

由Soong提供了androidmk工具进行mk文件到bp文件的转换

```
usage: androidmk [flags] <inputFile>

androidmk parses <inputFile> as an Android.mk file and attempts to output an analogous Android.bp file (to standard out)
```

例如：

```
androidmk Android.mk > Android.bp
```
- bp文件只包含一些简单的配置，因此一些与逻辑相关的或一些复杂的配置将不能转换成功，这些失败信息将会在转换后的bp文件中进行注释说明

#### 链接到共享库

- 在Android.mk中：

  ```
  LOCAL_SHARED_LIBRARIES += android.hardware.samples@1.0
  ```

- 在 **Android.bp** 中：

```
shared_libs: [
    /* ... */
    "android.hardware.samples@1.0",
]
```

#### 编译二进制可执行文件的模块

几个字段说明：

- cc_library对应于mk文件中module属性
- cc_defaults引入了继承的概念，表示父类即表示一个模块中的其他模块的父模块，通过制定defaults字段来继承父模块
- cc_binary表示将编译为二进制可执行文件

看一下cc_binary的用法示例

```
cc_binary {
    name: "dumpsys",

    defaults: ["dumpsys_defaults"],

    srcs: [
        "main.cpp",
    ],
}
```

其中：

- name表示二进制文件的名称
- defaults 此模块将继承名为dumpsys_defaults的父模块
- srcs表示执行此命令将被执行的文件

#### 更多字段说明

http://note.qidong.name/demo/soong_build/#collapse1733



