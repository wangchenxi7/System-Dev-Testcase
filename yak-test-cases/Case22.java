import java.util.HashSet;
import java.util.TreeSet;
import java.util.ArrayList;

//class MyObjectInRegion {
//}
//
//class MyObjectInHeap {
//}
//
public class Case22 {
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
    int index = 0;
    HashSet<Object> set  = new HashSet<Object>(4);
    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      for (int i=0; i<20; ++i) {
        //set.add(new MyObjectInRegion());
        set.add(new Object());
      }

      System.out.println("add objects done in region");

      sleep(1000);
      int remove_num = set.size() / 2;
//      for (int i=0; i<remove_num; ++i) {
//        set.remove(0);
//      }
      System.gc();
      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<10; ++i) {
        //set.add(new MyObjectInHeap());
        set.add(new Object());
      }
      System.gc();
      //for(Object o: set.toArray())
      for(Object o: set) {
        System.out.println(o);
      }
    }
  }

  public static void main(String args[]) {
    region_loop(10);
  }


}
