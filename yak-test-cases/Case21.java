import java.util.HashSet;
import java.util.ArrayList;

public class Case21 {
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
      //      System.gc();:: This is on in Case12
      Runtime.getRuntime().iterationStart();
      for (int i=0; i<1; ++i) {
        arrayList.add(new Object());
      }

      //System.out.println("add objects done in region");

      Object t = new Object();
      System.out.println("===> Step: 1");
      Object[] array = new Object[10];
      System.out.println("===> Step: 2");
      static_array = array;
      System.out.println("===> Step: 3");
      for (int i=0; i<array.length; ++i) {
        array[i] = new Object();
      }
      System.out.println("===> Step: 4");
      array[0] = t;
      System.out.println("===> Step: 5");
      HashSet<Object> set = new HashSet<Object>();
      System.out.println("===> Step: 6");
      for (int i=0; i<1; ++i) {
        set.add(new Object());
      }
      System.out.println("===> Step: 7");
      sleep(1000);
      System.out.println("===> Step: 8");
      int remove_num = arrayList.size() / 2;
      System.out.println("===> Step: 9");
      for (int i=0; i<remove_num; ++i) {
        arrayList.remove(i);
      }
      System.out.println("===> Step: 10");
      System.gc();
      System.out.println("===> Step: 11");
      Runtime.getRuntime().iterationEnd();
      System.out.println("===> Step: 12");

      for (int i=0; i<1; ++i) {
        arrayList.add(new Object());
      }
      
      System.out.println("===> Step: 13");
      System.gc(); // Root of failure - okay if this is off
      
      System.out.println(arrayList.size());
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}
