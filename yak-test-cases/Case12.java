import java.util.HashSet;
import java.util.ArrayList;

public class Case12 {
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
        Object o = new Object();
        System.out.println("inside " +o.toString());
        arrayList.add(o);
      }
      System.out.println();

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
      int remove_num = 5;//arrayList.size() / 2;
      for (int i=0; i<remove_num; ++i) {
        Object o= arrayList.remove(0);
         System.out.println("removed " + o.toString());
         System.out.println ("new 0 " +arrayList.get(0).toString());
      }
      System.out.println();
      // System.gc();
      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<10; ++i) {
        Object o = new Object();
        System.out.println("outside " +o.toString());

        arrayList.add(o);
        
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
