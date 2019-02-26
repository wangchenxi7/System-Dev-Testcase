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

public class Case32 extends Thread {
  public static Object[] static_array;
  
  public int loop_num = 10;
  public void my_sleep(long ms) {
    try {
      System.out.println("Go sleeping for " + ms/1000 + "!");
      this.sleep(ms);
      System.out.println("Wake up and done! Thread " + this.getId());
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }


  public void run() {
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
      System.out.println("Java region starts! Thread " + this.getId());

      if(count % 2 == 0){
        Node[] newNodes = new Node[nodes.length <<1];
        //reshuffle
	System.out.println("reshuffling...");
        for(int j = 0; j < nodes.length ; j++){
          Node m;
          for(Node n = nodes[j]; n != null; n = m){
            m = n.next;
            int newPos = n.value % newNodes.length;
	    // System.out.println("value: " + n.value + " pos: " + j + " new pos:" + newPos);
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

      System.out.println("add objects done in region, Thread " + this.getId());
      //my_sleep(1);
      System.gc();
      Runtime.getRuntime().iterationEnd();
      System.out.println("Java region ends! " + this.getId());

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
      // synchronized(this){
	for(int i = 0; i < nodes.length; i ++) {
	  Node n = nodes[i];
	  boolean newline=false;
	  while(n != null){
	    System.out.print(n.value+" " );
	    newline= true;
	    n = n.next;
	  }
	  if (newline) System.out.println();
	}
	// }
    }
  }

  public static void main(String args[]) {
    Thread[] threads = new Thread[10];
    for(int i = 0; i < 10; i++){
      threads[i] = new Case32();
      threads[i].start();
    }
    
    for(int i = 0; i < 10; i++){
      try{
      threads[i].join();
      }catch(Exception e){
	e.printStackTrace();
      }
    }

    
    System.out.println("Done!");
    
  }
}
