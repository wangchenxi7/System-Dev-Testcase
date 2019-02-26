import java.util.*;

class Node{
  Node next;
  Integer value;
  public Node(int v){
    value = v;
    next = null;
  }
}


public class Case29 {
  public static Node[] static_array;
  
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

    while (count++ < loop_num) {
            Runtime.getRuntime().iterationStart();
      System.out.println("Java region starts!");

      Node[] nodes = new Node[10];
      static_array = nodes;

      for(int i = 0; i < nodes.length; i++){
        nodes[i] = new Node(i);
        Node prev = nodes[i];
        for(int j = 0; j < 30; j++){
          Node n = new Node(i+j*25);
          prev.next = n;
          prev = n;
	  System.out.print(n.value+" ") ;
        }
      }
      System.out.println();

      System.out.println("add objects done in region");
      sleep(1);
      System.gc();

      System.out.println("Java region ends!");

      int sumx=0;
      for(int i = 0; i < 10; i++){
        Node tmp=nodes[i];
        while(tmp!=null) {
          sumx+=tmp.value;
          tmp=tmp.next;
        }
      }

      Runtime.getRuntime().iterationEnd();
      System.gc();


      System.out.println("SUMX="+sumx);
      System.out.println("count="+count);
      int sum=0;
      for(int i = 0; i < nodes.length; i ++) {
        Node n = nodes[i];
        boolean newline=false;
        while(n != null){
          sum+=n.value;
          System.out.print(n.value.toString()+" ");
          newline= true;
          n = n.next;
        }
        if (newline) System.out.println();
      }
      System.out.println("SUM="+sum);
      if (sum != 110145)
        throw new Error();
    }
  }

  public static void main(String args[]) {
    region_loop(80);
    System.out.println("Done!");
  }
}
