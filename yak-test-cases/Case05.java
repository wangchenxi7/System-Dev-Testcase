import java.util.HashSet;
import java.util.ArrayList;

public class Case05 {
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


  public static void test_region_1() {

    Runtime.getRuntime().iterationStart();
    Object t = new Object();
    System.gc();
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
    sleep(1000);
    Runtime.getRuntime().iterationEnd();


    ArrayList<Object> arrayList = new ArrayList<Object>();
    for (int i=0; i<100000; ++i) {
      arrayList.add(new Object());
    }

    Runtime.getRuntime().iterationStart();
    ArrayList<Object> list_1 = new ArrayList<Object>();
    
    {
      Runtime.getRuntime().iterationStart();
      list_1.add(new Object());
      Runtime.getRuntime().iterationEnd();
    }

    Runtime.getRuntime().iterationEnd();
    System.out.println(static_array.length);
  }

  public static void main(String args[]) {
    test_region_1();
  }
}
