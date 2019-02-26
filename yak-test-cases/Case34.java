import java.util.*;

// Test LargeObject and PageReuse

public class Case34 {
  
  class NodeCase34{

    int value;
    int ivalue;
    long lvalue;
    long llvalue;
    String svalue;

    public NodeCase34(int v){
      value = v;
      ivalue = v;
      lvalue = v<<2;
      llvalue = v <<3; 
      svalue = Integer.toString(v <<4);
    }
      
    public String toString() {
      return Integer.toString(value);
    }
  }

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


  public  void region_loop(int loop_num) {

    int count = 0;
    

    while (count < loop_num) {

       Runtime.getRuntime().iterationStart();
       NodeCase34[] seed = new NodeCase34[10000];
       for (int i = 0; i < seed.length; i++) {
         seed[i] = new NodeCase34(i);
       }


      System.gc();
      StringBuffer sb = new StringBuffer();
      for (int j = 0; j < seed.length; j++) {
        
        sb.append(seed[j] + " ");
      }
      System.out.println();

      Runtime.getRuntime().iterationEnd();
      System.gc();

      count++;

    }
  }
  
  
  public static void main(String args[]) {
    new Case34().region_loop(100);
  }
}


