import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Test the backbone watch mechanism.
* Test the System.arraycopy of object array.
*
* Structure:
* Big data array.
* 2 levels.
* Element is user defined Class objItem.
* Object Array Class Type is : [LobjItem;
*
* KeyPoint
* a. An object array only with arraycopy should be judged as non-backbone.
* b. Single Thread version. Spark do this in concurent. 
*
* Waining : line62's check is neccessary.
* Because the (obj) array.length can be null.
*/


public class SparkCase23{

  public static final int _elem_num = 4*1024*1024;         // 4M elements
  public static final int _small_elem_num = _elem_num/32;  // 128K elements


  public static void region_loop(int loop_num) {

    int count = 0;
    //Base object array used to as contents.
    objItem[] base_array = new objItem[_elem_num];
    //the array to be checked.
    objItem[] array = new objItem[_elem_num];
    //Initialize the base object array.
    for (int i= 0; i< _elem_num; ++i){
      base_array[i] = new objItem(i,2*i);
    }// end of for


    do{
	  // for loop#i, do #i+1 times arraycopy from base_array to array.
	  // for loop#i, do 1 times arraycopy from previous array to current array.
      System.out.println("Loop "+ count +", Phase 1 start");

      for (int i= 0; i< (count+1); ++i){
        if( (count+1)*_elem_num  > array.length){
          objItem[] new_array = new objItem[array.length + _elem_num];
          //copy the old content of this array.
          System.arraycopy(array,0, new_array,0, array.length);
          array = new_array;
        }
        //Add a base_array to its content.
        System.arraycopy(base_array,0, array,count*_elem_num, base_array.length);
      }// end of for

      //check the content of the array
      System.out.println("Check the contents of array.");
      for(int i=0; i<array.length;i++){
        if( i%(1024*1024)==0){
          if(array[i] == null)
			     System.out.println("array["+i+"] is null.");
		      else
			     System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val);
		    } //check object at 1024*1024 granulirity
      }

      //trigger minorGC
      HashSet<objItem> set = new HashSet<objItem>();
      System.out.println("New a HashSet to trigger minorGC.");

      for (int i=0; i<2*_elem_num; ++i) {
        set.add(new objItem(i,i*3));
      }


      System.out.println( "end of loop :" + count + " ,arrayList size :" + array.length);
      System.out.println("");
    }while(++count < loop_num); //end of while
  
    // Prevent the arraycopy is optimized by opt compiler
    System.out.println("");
    System.out.println("End of while, array.length:" +array.length);
    for(int i=0; i< array.length; i+= 3*1024*1024){
      if(array[i] != null){
        System.out.println("array["+i+"] :" + array[i].val);
      }else{
		    System.out.println("array["+i+"] is null.");
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



