import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Trigger minorGC by running out region space.
* But can't free enough space, so allocate recognized arrays into nomarl path.  
*
* Big data array.
* 2 levels.
* Element is user defined Class objItem.
* 
* Checks the content of the object array. //Compared to SparkTest10.java
*/


public class SparkCase11 {

  public static Object[] static_array;
  public static final int _elem_num = 8*1024*1024;    
  public static final int _small_elem_num = _elem_num/64;

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
    ArrayList<objItem> arrayList = new ArrayList<objItem>();
    while (count++ < loop_num) {
     
     System.out.println("Phase 1");
      for (int i=0; i< _elem_num; ++i) {
        arrayList.add(new objItem(i,i*2));        //when ArrayList scales, it build a new one & copy the contents.
      }

      //check the value of phase1

      for(int i=0; i< arrayList.size(); i++){
        if(i% (2*1024*1024) == 0)
          System.out.println("arrayList["+i+"] , key: " + arrayList.get(i).key + ", val : " + arrayList.get(i).val );
      }


      System.out.println("Phase 2");
      Object t = new Object();
      Object[] array = new Object[_small_elem_num];
      static_array = array;
      for (int i=0; i<array.length; ++i) {
        array[i] = new Object();
      }
      array[0] = t;

/*

      System.out.println("Phase 3");
      HashSet<Object> set = new HashSet<Object>();
      System.out.println("New a HashSet with default size.");

      for (int i=0; i<_elem_num; ++i) {
        set.add(new Object());
        
      //  if(i%1024 == 0)
      //    System.out.println("Add Element :" + i );
      }
*/

      System.out.println("Phase 4");
      for (int i=0; i<2; ++i) {
        arrayList.add(new objItem(-1,-1));
      }

      System.out.println( "end of loop :" + count + " ,arrayList size :" + arrayList.size());
      System.out.println("");
    }
  }





  public static void main(String args[]) {

    region_loop(5);
  }
}


class objItem{
  int  key;
  int  val;

  public objItem(int key, int val){
    this.key =key;
    this.val = val;
  }

}



