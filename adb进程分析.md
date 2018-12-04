issues:adb secure 的影响在哪里体现

### 加载默认USB属性

#### 编译期：
##### 编译生成persist.sys.usb.config属性

1. 在对应项目的配置文件中进行usb属性的配置

``` makefile
ifeq ($(TARGET_BUILD_VARIANT),user)
  PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.sys.usb.config=mtp
else
  PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.sys.usb.config=adb
endif
```

2. build\core\Makefile文件进行属性的解析

```makefile
# -----------------------------------------------------------------
# vendor default.prop
INSTALLED_VENDOR_DEFAULT_PROP_TARGET :=
ifdef property_overrides_split_enabled
#1.表明最终生成的文件在vendor\default.prop
INSTALLED_VENDOR_DEFAULT_PROP_TARGET := $(TARGET_OUT_VENDOR)/default.prop
ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_VENDOR_DEFAULT_PROP_TARGET)

#2.添加PRODUCT_DEFAULT_PROPERTY_OVERRIDES字段添加的属性
#2.1collapse-pairs函数去除空格
FINAL_VENDOR_DEFAULT_PROPERTIES += \
    $(call collapse-pairs, $(PRODUCT_DEFAULT_PROPERTY_OVERRIDES))
#2.2uniq-pairs-by-first-component过滤重复的属性设置，以第一个为准
FINAL_VENDOR_DEFAULT_PROPERTIES := $(call uniq-pairs-by-first-component, \
    $(FINAL_VENDOR_DEFAULT_PROPERTIES),=)

$(INSTALLED_VENDOR_DEFAULT_PROP_TARGET): $(INSTALLED_DEFAULT_PROP_TARGET)
	@echo Target buildinfo: $@
	@mkdir -p $(dir $@)
	$(hide) echo "#" > $@; \
	        echo "# ADDITIONAL VENDOR DEFAULT PROPERTIES" >> $@; \
	        echo "#" >> $@;
	#3.逐行遍历添加属性
	$(hide) $(foreach line,$(FINAL_VENDOR_DEFAULT_PROPERTIES), \
		echo "$(line)" >> $@;)
    #4.调用该脚本进行一些属性规则的限制，下文会进一步阐述
	$(hide) build/tools/post_process_props.py $@

endif  # property_overrides_split_enabled
```

3. 
4. 特别的说明

```makefile
BUILDINFO_SH := build/tools/buildinfo.sh
VENDOR_BUILDINFO_SH := build/tools/vendor_buildinfo.sh
```

在system\build.prop和vendor\build.prop文件生成的位置，上述两个脚本将会参与这个过程，将其中的属性值写入文件



#### 运行期：

1. init进程进行属性的加载system\core\init\property_service.cpp

2. 函数适配debuggable属性


```c++
// persist.sys.usb.config values can't be combined on build-time when property files are split into each partition.
// So we need to apply the same rule of build/make/tools/post_process_props.py on runtime.
static void update_sys_usb_config() {
    bool is_debuggable = android::base::GetBoolProperty("ro.debuggable", false);
    std::string config = android::base::GetProperty("persist.sys.usb.config", "");
    if (config.empty()) {
        property_set("persist.sys.usb.config", is_debuggable ? "adb" : "none");
    } else if (is_debuggable && config.find("adb") == std::string::npos &&
               config.length() + 4 < PROP_VALUE_MAX) {
        config.append(",adb");
        property_set("persist.sys.usb.config", config);
    }
}
```
3. 上层系统服务

### ADB授权

1. adbd进程和adb进程通信方式
2. 授权key以及预置adb_keys

### CTS检查

``` java
/cts/tests/tests/os/src/android/os/cts/UsbDebuggingTest.java
```

```java
public void testUsbDebugging() {
27        // Secure USB debugging must be enabled
28        assertEquals("1", SystemProperties.get("ro.adb.secure"));
29
30        // Don't ship vendor keys in user build
          // user版本不允许预置adb_keys在系统中
31        if ("user".equals(Build.TYPE)) {
32            File keys = new File("/adb_keys");
33            assertFalse(keys.exists());
34        }
35    }
```

