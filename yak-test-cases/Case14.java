import java.util.HashSet;
import java.util.ArrayList;

public class Case14 {
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
    Object[] array = new Object[100000];
    static_array = array;
    int index = 0;
    int size = 0;
    while (count++ < loop_num) {

      // This is the only difference from Case11
      //System.gc(); // This cause assertion failure
      // ==========================

      Runtime.getRuntime().iterationStart();
      //System.gc(); // moved the call above to here will also cause assertion failure
      for (int i=0; i<10; ++i) {
        Object o = new Object();
        System.out.println("inside " +o.toString());
        array[index++]=o;
        size++;
      }
      System.out.println();

      //System.out.println("add objects done in region");

      sleep(500);
      int remove_num = 5;
      for (int i=0; i<remove_num; ++i) {
        Object o= array[0];
        for (int j = 1; j < size; j++){
          array[j-1]=array[j];
        }
         System.out.println("removed " + o.toString());
         Object new_o = array[0];
         System.out.println ("new 0 " +new_o.toString());
         size--;
         index--;
      }
      System.out.println();
      // System.gc();
      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<10; ++i) {
        Object o = new Object();
        System.out.println("outside " +o.toString());

        array[index++]=o;
        size++;
      }
      
      //System.gc();

      System.out.println(size);
      for (int i =0; i < size;++i) {
        System.out.print(" "+i+":");
        System.out.print(array[i]);

      }
      System.out.println();
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}
