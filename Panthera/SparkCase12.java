import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Test the backbone watch mechanism.
* Test the System.arraycopy of object array.
* Test the object array store, aastore.
*
* Warnings : opt compiler may optimize the aastore to arraycopy.
* So, must confirm the write barrier check number.
* Can use command line below to skip the optimization.
* -XX:CompileCommand=exclude,SparkCase12.region_loop 
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


public class SparkCase12 {

  public static final int _elem_num = 8*1024*1024;         // 8M elements
  public static final int _small_elem_num = _elem_num/64;  // 128K elements


  public static void region_loop(int loop_num) {

    int count = 0;
    objItem[] array = new objItem[_small_elem_num];
    do{
      System.out.println("Loop "+ count +", Phase 1 start");

      for (int i= count*_elem_num; i< (count+1)*_elem_num; ++i){
        if(i >= array.length){
          objItem[] new_array = new objItem[array.length + _elem_num];
          System.arraycopy(array,0, new_array,0, array.length);
          array = new_array;
        }
        array[i] = new objItem(i,2*i);
      }// end of for

      //check the content of the array
      System.out.println("Check the contents of array.");
      for(int i=0; i<_elem_num;i++){
        if( i%(1024*1024)==0 )
          System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val);
      }

      System.out.println( "end of loop :" + count + " ,arrayList size :" + array.length);
      System.out.println("");
    }while(++count < loop_num); //end of while
  
    // Prevent the arraycopy is optimized by opt compiler
    System.out.println("");
    System.out.println("End of while, array.length:" +array.length);
    for(int i=0; i< array.length; i+= 4*1024*1024){
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



