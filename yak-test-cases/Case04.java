/*
 * Case 04: 
 * First part: -same as case 03:
 * Allocate an array of 10 objects in 2 regions. Single thread.  Container is created in parent region. 
 * Content is created in child region, which makes 10 objs escaping the child region
 * Second part is the duplication of the first part to see if the recycled pages are doing fine or not
 */

import java.lang.*;

public class Case04{

  public void run() {

    // First Part
    Runtime.getRuntime().iterationStart();

    Test[] arr = new Test[10];

    Runtime.getRuntime().iterationStart();
    for (int i = 0 ; i < arr.length; ++i ) {
      Test t = new Test(i);
      arr[i] = t;
    }
    Runtime.getRuntime().iterationEnd();
    
    for (int i = 0 ; i < arr.length; ++i ) {
      // System.out.print  (arr[i].toString()); //:: Turn this on will lead to asertion failure (case 02)Feb 29
      System.out.println(" " + arr[i].i);
    }

    Runtime.getRuntime().iterationEnd();

    //Second Part :: should use the pages from first part
    
    Runtime.getRuntime().iterationStart();

    Test[] array = new Test[10];

    Runtime.getRuntime().iterationStart();
    for (int i = 0 ; i < array.length; ++i ) {
      Test t = new Test(i);
      array[i] = t;
    }
    Runtime.getRuntime().iterationEnd();

    for (int i = 0 ; i < array.length; ++i ) {
      // System.out.print  (array[i].toString()); //:: Turn this on will lead to asertion failure (case 02)Feb 29
      System.out.println(" " + array[i].i);
    }

    Runtime.getRuntime().iterationEnd();

  }

  public static void main(String[] args){
    
    Case04 c = new Case04();
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



