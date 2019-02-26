import java.util.HashSet;
import java.util.ArrayList;

public class Case23 {
  public static Object[] static_array;

  public static void sleep(long ms) {
    try {
      System.out.println("Go sleeping for " + ms/1000 + "!");
      Thread.sleep(ms);
      System.out.println("Wake up and done!");
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }


  public static void region_loop(int loop_num) {

    int count = 0;

    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      HashSet<Object> set  = new HashSet<Object>();
      for (int i=0; i<10; ++i) {
        set.add(new Object());
      }

      System.out.println("add objects done in region");

      Object t = new Object();
      Object[] array = new Object[100];
      static_array = array;
      for (int i=0; i<array.length; ++i) {
        array[i] = new Object();
      }
      array[0] = t;
      sleep(1000);
      int remove_num = set.size() / 2;
      for (int i=0; i<remove_num; ++i) {
        set.remove(0);
      }

      for (int i=0; i<10; ++i) {
        set.add(new Object());
      }
      System.gc();
      for(Object o: set)
      System.out.println(o);
      Runtime.getRuntime().iterationEnd();
      System.gc();
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}
