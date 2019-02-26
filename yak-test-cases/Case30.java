import java.util.*;

class Node{
  Node next;
  Integer value;
  public Node(int v){
    value = v;
    next = null;
  }
}


public class Case30 {
  public static Node[] nodes;
  
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
    nodes = null;

    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      System.out.println("Java region starts!");

      nodes = new Node[10];
      
      for(int i = 0; i < nodes.length; i++){
        nodes[i] = new Node(i);
        Node prev = nodes[i];
        for(int j = 0; j < 30; j++){
          Node n = new Node(i+j*25);
          prev.next = n;
          prev = n;
        }
      }


      Runtime.getRuntime().iterationEnd();
      System.gc();
      System.out.println("count="+count);

      Node v = nodes[0];
      System.out.println("0");
      System.out.println(v);
      System.out.println(v.value);
      System.out.println(v.next);

    }
  }

  public static void main(String args[]) {
    region_loop(80);
    System.out.println("Done!");
  }
}
