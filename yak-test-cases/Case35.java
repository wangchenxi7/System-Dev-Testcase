import java.util.*;

class Case35Node{
  public Case35Node(int value){this.value = value;}
  int value;
}

class DummyThread extends Thread {

  public int region_id;
  public Case35Node[] array;

  public DummyThread(int r, Case35Node[] arr) {
    region_id = r;
    array = arr;
  }

  public void set_rid (int r) {
    region_id = r;
  }


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
    Runtime.getRuntime().createRegion(region_id);
    for(int i = 0; i < 10; i++){
      array[((int)Thread.currentThread().getId())*10 + i] = new Case35Node(i * 125 + 6);
      // System.out.println(array[i]);
    }
   
    System.gc();
    Runtime.getRuntime().deleteRegion(region_id);
    // my_sleep(500);
    System.gc();
  }

}

public class Case35 {
  
  

  public void run(int loop_num) {
    int count = 0;

    while (++count < loop_num) {

      System.out.println ("****** iteration  " + count + " ****************");
      int id = Runtime.getRuntime().iterationStart();      
      Case35Node[] array = new Case35Node[2000];
      Thread[] threads = new Thread[10];

      for(int i = 0; i < 10; i++){
        threads[i] = new DummyThread(id, array);
        threads[i].start();
      }

      for(int i = 0; i < 10; i++){
        try{
          threads[i].join();
          threads[i] = null;
        }catch(Exception e){
          e.printStackTrace();
        }
      }

      


      Runtime.getRuntime().iterationEnd(); 
    }

  }

  public static void main(String args[]) {
   
    int loop = 10; //default
    if (args.length == 1) {
      loop = Integer.parseInt(args[0]);
    }
    
    new Case35().run(loop);
    
    System.out.println("Done!");
    
  }
}
