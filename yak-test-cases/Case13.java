import java.util.HashSet;
import java.util.ArrayList;

// No force GC, Minor only, validate the content of the array

public class Case13 {
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

      // This is the only difference from Case11
      //System.gc(); // This cause assertion failure
      // ==========================

      Runtime.getRuntime().iterationStart();
      //System.gc(); // moved the call above to here will also cause assertion failure
      for (int i=0; i<10; ++i) {
        arrayList.add(new Object());
      }

      //System.out.println("add objects done in region");

      Object t = new Object();
      Object[] array = new Object[10];
      static_array = array;
      for (int i=0; i<array.length; ++i) {
        array[i] = new Object();
      }
      array[0] = t;
      HashSet<Object> set = new HashSet<Object>();
      for (int i=0; i<10; ++i) {
        set.add(new Object());
      }
      sleep(500);
      int remove_num = arrayList.size() / 2;
      for (int i=0; i<remove_num; ++i) {
        arrayList.remove(i);
      }
      // System.gc();
      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<10; ++i) {
        arrayList.add(new Object());
      }
      
      //System.gc();

      System.out.println(arrayList.size());
      for (int i =0; i < arrayList.size();++i) {
        System.out.print(" "+i+":");
        System.out.print(arrayList.get(i));

      }
      System.out.println();
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}
