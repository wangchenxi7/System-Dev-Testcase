import java.util.*;




public class Case33 {
  
  class NodeCase33{

    int value;

    public NodeCase33(int v){
      value = v;
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
    
    NodeCase33[][] matrix = new NodeCase33[300][];


    while (count < loop_num) {
      NodeCase33[] seed = new NodeCase33[20];
      for (int i = 0; i < seed.length; i++) {
        seed[i] = new NodeCase33(i);
      }
       Runtime.getRuntime().iterationStart();
      NodeCase33[] copy = Arrays.copyOf(seed, 10);
      for (int i = 0; i < 10; i ++) {
        copy[i].value += count*25;
      }
      matrix[count]  = copy;
      System.gc();
       Runtime.getRuntime().iterationEnd();
      
      for (int i = 0; i <= count; i++) {
        NodeCase33[] arr = matrix[i];
        System.out.print ("Elem " + i + " : ");
        for (int j = 0; j < 10; j++) {
          System.out.print (arr[j] + " ");
            
        }
        System.out.println();
      }
      count++;
      System.gc();
    }
  }
  
  
  public static void main(String args[]) {
    new Case33().region_loop(100);
  }
}


