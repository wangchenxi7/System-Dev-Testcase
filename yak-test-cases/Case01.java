/*
 * Case 01: Allocate 01 object in region. Only 1 region. Single thread.  
 */

import java.lang.*;

public class Case01{

  public void run() {
    
    Runtime.getRuntime().iterationStart();
    Test t = new Test();
    Runtime.getRuntime().iterationEnd();
    System.out.println(t.toString());
  }

  public static void main(String[] args){
    
    Case01 c = new Case01();
    c.run();
    System.out.println("Done!");

  }

  class Test {
    int i;
    public Test() {
      i = 0;
    }
  }

}



