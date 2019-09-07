import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* a. MinorGC thread gather object array's element into a region.
*	   So we first test small object array.
* b. Test sub-backbone region mechanism
*    This region is read only but referenced by other regions.
* c. Test stop optimization:
*    c.1 if region space runs out, stop all the closure work 
*    c.2 if there is a in-validate target object, stop this region's closure work.     
*
* Structure:
* Big data array.
* 2 levels.
* Element is user defined Class objItem.
* Object Array Class Type is : [LobjItem;
* 
* Waining : line62's check is neccessary.
* Because the (obj) array.length can be null.
*/


public class SparkCase30 {

  public static final int _elem_num = 8*1024;             // 8k elements
  public static final int _small_elem_num = _elem_num/8;  // 128 elements


  public static void region_loop(int loop_num) {

    int count = 0;
    objItem[] array = new objItem[_small_elem_num];			                      //Sub backbone region.
    do{
      System.out.println("Loop "+ count +", Phase 1 start");

      for (int i= count*_elem_num; i< (count+1)*_elem_num; ++i){
        if(i >= array.length){
          objItem[] new_array = new objItem[array.length + _small_elem_num];  // Generate many sub-backbone region. No aastore in next loop.
          System.arraycopy(array,0, new_array,0, array.length);
          array = new_array;
        }
        array[i] = new objItem(i,2*i);
      }// end of for

	  //Trigger minorGC & fullGC manually to do closure work.
	  System.out.println("Trigger minorGC & fullGC manually to do closure work.");
	  System.gc();

      //check the content of the array
      System.out.println("Check the contents of array.");
      for(int i=0; i<_elem_num;i++){
        if( i%(1024)==0 )
          System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val);
      }

      System.out.println( "end of loop :" + count + " ,arrayList size :" + array.length);
      System.out.println("");
    }while(++count < loop_num); //end of while
  
    // Prevent the arraycopy is optimized by opt compiler
    System.out.println("");
    System.out.println("End of while, array.length:" +array.length);
    for(int i=0; i< array.length; i+= 4*1024){
      if(array[i] != null){
        System.out.println("array["+i+"] :" + array[i].val);
      }
    }

    System.out.println("");
    System.out.println("****************PASSED****************");
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



