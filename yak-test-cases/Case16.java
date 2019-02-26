import java.util.HashSet;
import java.util.ArrayList;

public class Case16 {
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

    int total =0;
    int count = 0;
    ArrayList<MyObject> arrayList = new ArrayList<MyObject>(12000);
    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      for (int i=0; i<10; ++i) {
        arrayList.add(new MyObject(++total));
      }

      //System.out.println("add objects done in region");

      int remove_num = 0;//arrayList.size() / 2;
      for (int i=0; i<remove_num; ++i) {
        arrayList.remove(0);
      }

      System.gc();
      Runtime.getRuntime().iterationEnd();

      for (int i=0; i<10; ++i) {
        arrayList.add(new MyObject(++total));
      }
      System.gc();
      for (int i=0; i <arrayList.size(); ++i) {
        System.out.print(" "+i+":"+arrayList.get(i).value());
      }
      System.out.println();
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}

class MyObject{
    
  int i = 0;
  int j = 0;
  public MyObject(int v) {
    i = v;
    j = v+1;
  }
  
  public int value(){
    return i;
  }
  
}

