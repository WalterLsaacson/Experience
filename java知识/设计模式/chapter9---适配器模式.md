### 适配器模式

* 现有的类或对象不满足新环境要求的接口

- 理解：将现有类或接口进行包装，在其他类使用当前适配器时就像是使用要求的新的接口一样，而其实在适配器内部是调用旧类或接口的一个或多个接口来实现该行为

```java
public interface MediaPlayer{
    public void play(String audioType, String fileName);
}

public interface AdvanceMediaPlayer{
    public void playVlc(String fileName);
    public void playMp4(String fileName);
}
/*Double interfers to imitate adapter mode*/

public class VlcPlayer implements AdvanceMediaPlayer{
    @Override
    public void playVlc(String fileName){
        Syso("Playing vlc file.Name: " + fileName);
    }
    @Override
    public void playMp4(String fileName){
        //do nothing
    }
}

public class Mp4Player implements AdvanceMediaPlayer{
    @Override
    public void playVlc(String fileName){
        //do nothing
    }
    @Override
    public void playMp4(String fileName){
        Syso("Playing mp4 file.Name: " + fileName);
    }
}
/*Different audio type players*/

public class MediaAdapter implements MediaPlayer{
    
    AdvanceMediaPlayer advanceMusicPlayer;
    
    public MediaAdapter(String audioType){
        if(audioType.equalsIgnoreCase("vlc")){
            advanceMusicPlayer = new VlcPlayer();
        } else if (audioType.equalsIgnoreCase("mp4")){
            advanceMusicPlayer = new Mp4Player();
        }
    }
    
   @Override
   public void play(String audioType, String fileName) {
      if(audioType.equalsIgnoreCase("vlc")){
         advancedMusicPlayer.playVlc(fileName);
      }else if(audioType.equalsIgnoreCase("mp4")){
         advancedMusicPlayer.playMp4(fileName);
      }
   }
}

public class AudioPlayer implements MediaPlayer {
   MediaAdapter mediaAdapter; 
 
   @Override
   public void play(String audioType, String fileName) {    
 
      //播放 mp3 音乐文件的内置支持
      if(audioType.equalsIgnoreCase("mp3")){
         System.out.println("Playing mp3 file. Name: "+ fileName);         
      } 
      //mediaAdapter 提供了播放其他文件格式的支持
      else if(audioType.equalsIgnoreCase("vlc") 
         || audioType.equalsIgnoreCase("mp4")){
         mediaAdapter = new MediaAdapter(audioType);
         mediaAdapter.play(audioType, fileName);
      }
      else{
         System.out.println("Invalid media. "+
            audioType + " format not supported");
      }
   }   
}

public class AdapterPatternDemo {
   public static void main(String[] args) {
      AudioPlayer audioPlayer = new AudioPlayer();
 
      audioPlayer.play("mp3", "beyond the horizon.mp3");
      audioPlayer.play("mp4", "alone.mp4");
      audioPlayer.play("vlc", "far far away.vlc");
      audioPlayer.play("avi", "mind me.avi");
   }
}

```



