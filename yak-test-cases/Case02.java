/*
 * Case 02: Allocate an array of 10 objects in region. Only 1 region. Single thread.  
 */

import java.lang.*;

public class Case02{

  public void run() {


    Runtime.getRuntime().iterationStart();

    Test[] arr = new Test[10];

    for (int i = 0 ; i < arr.length; ++i ) {
      Test t = new Test(i);
      arr[i] = t;
    }


    for (int i = 0 ; i < arr.length; ++i ) {
      System.out.print  (arr[i].toString()); //:: Turn this on will lead to asertion failure Feb 29
      System.out.println(" " + arr[i].i);
    }

    Runtime.getRuntime().iterationEnd();
  }

  public static void main(String[] args){
    
    Case02 c = new Case02();
    c.run();
    System.out.println("Done!");
    
  }

  class Test {
    public int i;
   
    public Test() {
      i = 0;
    }
    
    public Test(int i) {
      this.i = i;
    }
  
  }

}



