import java.util.HashSet;
import java.util.ArrayList;

public class Case07 {
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
    ArrayList<Object> arrayList = new ArrayList<Object>();
    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      for (int i=0; i<1000; ++i) {
        arrayList.add(new Object());
      }

      Object t = new Object();
      Object[] array = new Object[100];
      static_array = array;
      for (int i=0; i<array.length; ++i) {
        array[i] = new Object();
      }
      array[0] = t;
      HashSet<Object> set = new HashSet<Object>();
      for (int i=0; i<1000; ++i) {
        set.add(new Object());
      }
      System.out.println("add objects done in region");

  //    sleep(1000);
      int remove_num = arrayList.size() / 2;
      for (int i=0; i<remove_num; ++i) {
        arrayList.remove(i);
      }

      System.out.println("remove objects done in region");

      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<1000; ++i) {
        arrayList.add(new Object());
      }
      System.out.println(arrayList.size());
    }
  }





  public static void main(String args[]) {
    region_loop(100);
  }
}
