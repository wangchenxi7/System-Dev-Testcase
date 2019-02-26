import java.util.*;

class Node{
  Node next;
  Integer value;
  public Node(int v){
    value = v;
    next = null;
  }
}

class MyHead{
  Node[] array;
}

public class Case26 {
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
    Node[] nodes = new Node[10];
    MyHead head = new MyHead();
    head.array = nodes;
    for(int i = 0; i < nodes.length; i++){
      nodes[i] = new Node(i);
      Node prev = nodes[i];
      for(int j = 0; j < 30; j++){
        Node n = new Node(j*25);
        prev.next = n;
        prev = n;
      }
    }

    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      System.out.println("Java region starts!");

      if(count % 2 == 0){
        Node[] newNodes = new Node[nodes.length <<1];
        //reshuffle
        for(int j = 0; j < nodes.length ; j++){
          Node m;
          for(Node n = nodes[j]; n != null; n = m){
            m = n.next;
            int newPos = n.value % newNodes.length;
            Node ntail = newNodes[newPos];
            if(ntail == null)newNodes[newPos] = n;
            else{
              Node next = ntail.next;
              while(next != null){
                ntail = next;
                next = ntail.next;
              }
              ntail.next = n;
            }
            n.next = null;
          }
        }
        nodes = newNodes;
        head.array = nodes;
      }

      for(int i = 0; i < 10; i++){
        int pos = (i * 156) % nodes.length;
        Node ntail = nodes[pos];
        if(ntail == null) nodes[pos] = new Node(i);
        else{
          Node next = ntail.next;
          while(next != null){
            ntail = next;
            next = ntail.next;
          }
          ntail.next = new Node(i);
        }	
      }

      System.out.println("add objects done in region");
      sleep(1);
      System.gc();
      Runtime.getRuntime().iterationEnd();
      System.out.println("Java region ends!");



      for(int i = 0; i < 10; i++){
        int pos = (i * 156) % nodes.length;
        Node ntail = nodes[pos];
        Node next = ntail.next;
        if(ntail == null) nodes[pos] = new Node(i);
        else{
          while(next != null){
            ntail = next;
            next = ntail.next;
          }
          ntail.next = new Node(i);
        }
      }

      System.gc();
      //for(Object o: set.toArray())
      for(int i = 0; i < nodes.length; i ++) {
        Node n = nodes[i];
        boolean newline=false;
        while(n != null){
          System.out.print(n.value+" ");
          newline= true;
          n = n.next;
        }
        if (newline) System.out.println();
      }
    }
  }

  public static void main(String args[]) {
    region_loop(10);
    System.out.println("Done!");
  }
}
