import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Test the backbone watch mechanism.
* Utilize Interpreter/C1/C2 to insert all the field addr into RDDTlab_entry->_ref_queue
*
* Structure:
* Big data array.
* 3 levels : objItem array --> objItem Instance --> internalItem instance
* Element is user defined Class objItem, interalItem.
* Object Array Class Type is : [LobjItem;
* 
*
* KeyPoint : source obj.field --> destination obj
* a. All Interpreter/C1/C2 should add the decoded field addr(oop*) into RDDTlab_entry->_ref_queue.
*    Not the source object addr
* b. Both object fields and object array element should be checked.
* c. Only test field addr recording via compiler. Not test field addr record via GC. Not test element gathering.
*
* Waining : "array[i] == null" check is neccessary.
* Because the (obj) array.length can be null.
*/


public class SparkCase20 {

  public static final int _elem_num = 4*1024*1024;            // 4M elements
  public static final int _small_elem_num = _elem_num/128;    // 32K elements

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

    //the array to be checked.
    objItem[] array = new objItem[_small_elem_num];

    do{ 
      // 1) Check object array assignment : aastore
      System.out.println("Loop "+ count +", Phase 1 start : Do object array element assignment, aastore \n");

      // extand the object arra
      if( (count+1)*_small_elem_num  > array.length ){
        objItem[] new_array = new objItem[array.length + _small_elem_num];
        //copy the old content of this array.
        System.arraycopy(array,0, new_array,0, array.length);
        array = new_array;
      }

      // do aastore
      for(int i= count*_small_elem_num; i< (count+1)*_small_elem_num; i++ ){
        array[i] = new objItem(i, 2*i);
      }

      // sleep 2s
      sleep(2000);

      // 2) do object field assignment : putfield

      System.out.println("Loop "+ count +", Phase 2 start : Do object field assignment, putfield \n");
      for(int i=count*_small_elem_num; i< (count+1)*_small_elem_num; i++ ){
        array[i].field_1 = new internalItem(i);
        array[i].field_2 = new internalItem(2*i);
        array[i].field_3 = new internalItem(3*i);
        array[i].field_4 = new internalItem(4*i);
      }


      //check the content of the array
      System.out.println("Check the contents of array.");
      for(int i=0; i<array.length;i++){
        if( i%(array.length/8)==0){
          if(array[i] == null)
			     System.out.println("array["+i+"] is null.");
		      else
			     System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val + ", internalItem field_3 id:" + array[i].field_3.id);
		    } //check object at 1024*1024 granulirity
      }


      System.out.println( "end of loop :" + count + " ,arrayList size :" + array.length);
      System.out.println("");
    }while(++count < loop_num); //end of while
  
    // Prevent the arraycopy is optimized by opt compiler
    System.out.println("");
    System.out.println("End of while, array.length:" +array.length);
    for(int i=0; i< array.length; i+= array.length/8 ){
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

 public int  key;
 public int  val;

  // add some fields 
  internalItem field_1;
  internalItem field_2;
  internalItem field_3;
  internalItem field_4;


  public objItem(int key, int val){
    this.key =key;
    this.val = val;
  }

}


class internalItem{
  int id;

  public internalItem(int id){
    this.id = id;
  }
}






