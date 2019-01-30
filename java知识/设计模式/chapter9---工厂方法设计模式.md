* 使用接口实现工厂设计模式

```java
interface Game{
    boolean move();
}
interface GameFactory{
    Game getGame();
}

class Checkers implements Game {
    private int moves = 0;
    private static final int MOVES = 3;
    public boolean move() {
        print("Checkers move " + moves);
        return ++move != MOVES;
    }
}

class CheckerFactory implements GameFactory {
    public Game getGame(){
        return  new Checkers();
    }
}

class Chess implements Game {
    private int moves = 0;
    private static final int MOVES = 4;
    public boolean move (){
        print("Chess move " + moves);
        return ++moves != MOVES;
    }
}

class ChessFactory implemetns GameFactory {
    public Game getGame(){
        return new Chess();
    }
}

public class Games{
    public static playGame(GameFactory factory){
        Game s = factory.getGame();
        while(s.move);
    }
    public static void main(String[] args){
        //different factorys different games be maked
        //anon class to replace multi factorys
        playGame(new CheckersFactory());
        playGame(new ChessFactory());
    }
}
```



* 优化使用匿名内部类实现工厂模式