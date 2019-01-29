### 策略设计模式

* 基于多态，即多个子类重写父类相同方法，但是有自己的不同实现
* 根据所传递的参数对象的 不同而具有不同行为的方法

```java
class Processor{
    public String name(){
        return getClass().getSimpleName();
    }
    Object process(Object input) {
        return input;
    }
}

class Upcase extends Processor{
    String process(Object input){  //Covariant return
        return ((String)input).toUpperCase();
    }
}

class Downcase extends Processor{
    String process(Object input){
        return ((String)input).toLowerCase();
    }
}

class Splitter extends Processor{
    String process(Object input){
        //The split argument divides aString into pieces
        return Arrays.toString(((String)input).split(""));
    }
}

public class Apply{
    public static void process(Processor p, Object s){
        print("Using Processor " + p.name());
        print(p.process(s));
    }
    public static String s = 
        "Disagreement with beliefs is by definition in corrent";
    public static void main(String[] args){
        // differt parameters,different behaviors
        process(new Upcase(), s);
        process(new Downcase(), s);
        process(new Splitter(), s);
    }
}
```

* 策略就是指每个子类中实现的不同逻辑
* 具有可扩展，可变的特点

### 优化

* 解耦---使用接口实现而不是类继承
