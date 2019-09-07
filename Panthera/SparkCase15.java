import java.util.HashSet;
import java.util.ArrayList;
import sun.misc.Unsafe;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class SparkCase15 {
  public static Object[] static_array;
  public static sun.misc.Unsafe unsafe;

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
    ArrayList<Object> arrayList = new ArrayList<Object>(1000);
    while (count++ < loop_num) {
      //      System.gc();:: This is on in Case12

      unsafe.enter_rdd_region();

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

      sleep(1000);
      int remove_num = 5;//arrayList.size() / 2;
      for (int i=0; i<remove_num; ++i) {
          arrayList.remove(0);
      }

      //System.gc();


      for (int i=0; i<10; ++i) {
        arrayList.add(new Object());
      }
      
      unsafe.leave_rdd_region();
      System.gc(); // Root of failure - okay if this is off

      for (int i=0; i<arrayList.size(); ++i) {
     //   System.out.print(i+":"+arrayList.get(i));
      }
      

    }//while
  }

  public static void main(String args[]) {

    try {
      Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
      unsafeField.setAccessible(true);
      unsafe = (sun.misc.Unsafe) unsafeField.get(null);
    } catch (Throwable cause) {
      unsafe = null;
    }

    region_loop(3);
  }
}
