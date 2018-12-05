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

2. build\core\Makefile文件进行mk文件的解析及相关属性文件的生成

```makefile
# -----------------------------------------------------------------
# vendor default.prop 以生成此文件为例
INSTALLED_VENDOR_DEFAULT_PROP_TARGET :=
ifdef property_overrides_split_enabled
#1.表明最终生成的文件在vendor\default.prop
INSTALLED_VENDOR_DEFAULT_PROP_TARGET := $(TARGET_OUT_VENDOR)/default.prop
ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_VENDOR_DEFAULT_PROP_TARGET)

#2.添加PRODUCT_DEFAULT_PROPERTY_OVERRIDES字段添加的属性
#2.1collapse-pairs函数去除空格
FINAL_VENDOR_DEFAULT_PROPERTIES += \
    $(call collapse-pairs, $(PRODUCT_DEFAULT_PROPERTY_OVERRIDES))
#2.uniq-pairs-by-first-component过滤重复的属性设置，以首次设置为准
FINAL_VENDOR_DEFAULT_PROPERTIES := $(call uniq-pairs-by-first-component, \
    $(FINAL_VENDOR_DEFAULT_PROPERTIES),=)

$(INSTALLED_VENDOR_DEFAULT_PROP_TARGET): $(INSTALLED_DEFAULT_PROP_TARGET)
	@echo Target buildinfo: $@
	#创建文件
	@mkdir -p $(dir $@)
	#文件头
	$(hide) echo "#" > $@; \
	        echo "# ADDITIONAL VENDOR DEFAULT PROPERTIES" >> $@; \
	        echo "#" >> $@;
	#3.逐行遍历添加属性
	$(hide) $(foreach line,$(FINAL_VENDOR_DEFAULT_PROPERTIES), \
		echo "$(line)" >> $@;)
    #4.调用该脚本进行一些属性规则的限制，其中会根据debuggable状态进行usb属性的配置，下文会详细阐述
	$(hide) build/tools/post_process_props.py $@

endif  # property_overrides_split_enabled
```

3. build/tools/post_process_props.py脚本进行USBdebug版本配置

```python
# Put the modifications that you need to make into the /system/etc/prop.default into this function. The prop object has get(name) and put(name,value) methods.
def mangle_default_prop(prop):
  #主要是在USBdebug模式下，会配置ro.debuggable属性为1，则将adb调试默认打开
  # If ro.debuggable is 1, then enable adb on USB by default
  # (this is for userdebug builds)
  if prop.get("ro.debuggable") == "1":
    val = prop.get("persist.sys.usb.config")
    if "adb" not in val:
      if val == "":
        val = "adb"
      else:
        val = val + ",adb"
      prop.put("persist.sys.usb.config", val)
  # UsbDeviceManager expects a value here.  If it doesn't get it, it will
  # default to "adb". That might not the right policy there, but it's better
  # to be explicit.
  #这里意思是如果本身没有配置persist.sys.usb.config属性，我们将它配置为调试模式的机制是不可取的
  if not prop.get("persist.sys.usb.config"):
    prop.put("persist.sys.usb.config", "none");
```

这个脚本只对debug版本生效，user版本不必关注

特别的说明：

```makefile
BUILDINFO_SH := build/tools/buildinfo.sh
VENDOR_BUILDINFO_SH := build/tools/vendor_buildinfo.sh
属性系统中生成vendor/build.prop和system/build.prop的主要内容
```

到此编译期的工作就做完了，将生成usb相关配置属性persist.sys.usb.config到vendor/default.prop文件中

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

