* #### java有几种基本类型
  * 八种 char,byte,short,int,long,float,double,boolean

* #### 自动装箱和拆箱
  * 基本数据类型均有对应的包装类
  * 自动装箱调用包装类的valueOf()方法，自动拆箱调用包装器的XXXValue方法
    * 包装类内部存储了一个对应数据类型的基本类型数值
    * 对于Integer包装类，如果数值在[-128,127]之间将直接从cache中取已经存在的对象
    * Integer、Short、Byte、Character、Long这几个类的valueOf方法的实现是类似的，均是有一个缓冲区
    * 对于Boolean类型来说则是提前缓冲好的两个TRUE、FALSE对象
    * 每个包装类的equals方法会先检查参数的类型，类型不对直接返回false

* #### String、StringBuffer和StringBuilder的区别
  * 均是final类，不可拓展
  * StringBuffer线程安全，内部使用synchronized关键字来保证
  * 大部分情况下java编译器会将String类的拼接操作翻译为StringBuilder类的append操作

* #### switch case将从特定case开始执行，确实break的情况下default也会执行

* #### 声明了有参构造方法之后，系统不会提供默认的无参构造方法

* #### 关于HashMap和HashTable的相关知识点

  * 