import java.util.*;

public class Case28 {
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
    RedBlackBST<Integer, String> tree = new RedBlackBST<Integer, String>();
    for(int i = 0; i < 25; i++){
      if(i%2 == 0){
	tree.put(new Integer(i+1), new Integer((i+1)*123).toString());
      }else{
	tree.put(new Integer(i), new Integer((i)*123).toString());
      }
    }

    while (count++ < loop_num) {
      Runtime.getRuntime().iterationStart();
      System.out.println("Java region starts!");

      if(count % 2 == 0){
	tree.put(new Integer(count), new Integer((count)*123).toString());
      }else{
	tree.put(new Integer(count+1), new Integer((count+1)*623).toString());
      }

      tree.check();
      System.out.println("add objects done in region");
      // System.gc();
      tree.check();

      for(Integer k: tree.keys()){
	String v = tree.get(k);
	System.out.print("key = " + k + " value = " + v);
      }

      Runtime.getRuntime().iterationEnd();
      tree.check();
      System.out.println("Java region ends!");
      for(int i = 25; i < 60; i++){
	if(i%2 == 0){
	  tree.put(new Integer(i+1), new Integer((i+1)*123).toString());
	}else{
	  tree.put(new Integer(i), new Integer((i)*123).toString());
	} 
      }   
      tree.check();

      System.gc();
      tree.check();
    }
  }

  public static void main(String args[]) {
    region_loop(100);
  }
}
