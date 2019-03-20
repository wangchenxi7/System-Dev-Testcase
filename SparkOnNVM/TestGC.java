import java.util.HashSet;
public class TestGC {

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
    array[0] = t;
    HashSet<Object> set = new HashSet<Object>();
    for (int i=0; i<1; ++i) {
      set.add(new Object());
    }
    sleep(1000);
    Runtime.getRuntime().iterationEnd();
    System.out.println(t.toString());
    System.out.println(array.toString());
  }

  // Output information when System.gc() is called.
  public static void test_system_gc() {
    System.out.println("Call System.gc()!");
    System.gc();
    sleep(10000);
  }

  public static void test_triggered_gc() {
    System.out.println("Try to trigger the gc, NOT from System.gc()!");
    HashSet<Object> set = new HashSet<Object>();
    for (int i=0; i<1000; ++i) {
      set.add(new Object());
    }
    sleep(10000);
  }

  public static void main(String args[]) {
    // test_system_gc();
    // test_triggered_gc();
    test_region_1();
  }
}
