import java.util.*;
class NodeCase27{
  NodeCase27 next;
  int value;
  public NodeCase27(int v){
    value = v;
    next = null;
  }
}
class MyHead{
  NodeCase27[] array;
}

public class Case27 {
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
    NodeCase27[] nodes = new NodeCase27[10];
    MyHead head = new MyHead();
    head.array = nodes;
    for(int i = 0; i < nodes.length; i++){
      nodes[i] = new NodeCase27(i);
      NodeCase27 prev = nodes[i];
      for(int j = 0; j < 30; j++){
        NodeCase27 n = new NodeCase27(j*25);
        prev.next = n;
        prev = n;
      }
    }

    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      System.out.println("Java region starts!");

      if(count % 2 == 0){
        NodeCase27[] newNodeCase27s = new NodeCase27[nodes.length <<1];
        //reshuffle
        for(int j = 0; j < nodes.length ; j++){
          NodeCase27 m;
          for(NodeCase27 n = nodes[j]; n != null; n = m){
            m = n.next;
            int newPos = n.value % newNodeCase27s.length;
            NodeCase27 ntail = newNodeCase27s[newPos];
            if(ntail == null)newNodeCase27s[newPos] = n;
            else{
              NodeCase27 next = ntail.next;
              while(next != null){
                ntail = next;
                next = ntail.next;
              }
              ntail.next = n;
            }
            n.next = null;
          }
        }
        nodes = newNodeCase27s;
        head.array = nodes;
      }

      for(int i = 0; i < 10; i++){
        int pos = (i * 156) % nodes.length;
        NodeCase27 ntail = nodes[pos];

        if(ntail == null) nodes[pos] = new NodeCase27(i);
        else{
          NodeCase27 next = ntail.next;
          while(next != null){
            ntail = next;
            next = ntail.next;
          }
          ntail.next = new NodeCase27(i);
        }	
      }

      System.out.println("add objects done in region");
      //sleep(1000);
      System.gc();
      Runtime.getRuntime().iterationEnd();
      System.out.println("Java region ends!");



      for(int i = 0; i < 10; i++){
        int pos = (i * 156) % nodes.length;
        NodeCase27 ntail = nodes[pos];
        NodeCase27 next = ntail.next;
        if(ntail == null) nodes[pos] = new NodeCase27(i);
        else{
          while(next != null){
            ntail = next;
            next = ntail.next;
          }
          ntail.next = new NodeCase27(i);
        }
      }

      System.gc();
      //for(Object o: set.toArray())
      for(int i = 0; i < nodes.length; i ++) {
        NodeCase27 n = nodes[i];
        while(n != null){
          System.out.println(n.value);
          n = n.next;
        }
      }
    }
  }

  public static void main(String args[]) {
    region_loop(20);
  }
}
