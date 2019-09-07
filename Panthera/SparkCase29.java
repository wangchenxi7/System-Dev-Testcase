import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Test card table & remembered set mechanism. 
* 1) Same as SparkCase28, but this case concerntrate on full-gc.
*
* Structure:
* Big data array.
* 3 levels : objItem array --> objItem Instance --> internalItem instance
* Element is user defined Class objItem, interalItem.
* Object Array Class Type is : [LobjItem;
* 
*
* KeyPoint : 
* a. Utilize both card table & remembered set to maintain RDDTlab_entry's ougoing reference.
* b. Do Element Gathering via minorGC.
* c. If any element object can't be moved into corresponding RDDTlab_entry, add &field into RDDTlab_entry->_ref_queue. 
*
* Waining : "array[i] == null" check is neccessary.
* Because the (obj) array.length can be null.
*/


public class SparkCase29 {

  public static final int _elem_num = 4*1024*1024;            // 4M elements
  public static final int _small_elem_num = _elem_num/4096;   // 1K elements




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

    // object array used for prevent elements move-in operation
    objItem[] disturb_array = new objItem[257];                  //why not promote any element objects into to_space ??
    //debug
//    System.out.println("disturb_array.leng "+ disturb_array.length);

    // the array to be checked.
    objItem[] array = new objItem[_elem_num];

    do{ 
      // 1) Check object array assignment : aastore
      System.out.println("Loop "+ count +", Phase 1 start : Do object array element assignment, aastore \n");

      // extand the object arra
      if( (count+1)*_elem_num  > array.length ){
        objItem[] new_array = new objItem[array.length + _elem_num];
        //copy the old content of this array.
        System.arraycopy(array,0, new_array,0, array.length);
        array = new_array;
      }

      // do aastore
      for(int i= count*_elem_num; i< (count+1)*_elem_num; i++ ){
        array[i] = new objItem(i, 2*i);
      }

      // sleep 2s
      sleep(2000);

      // 2) do object field assignment : putfield
      System.out.println("Loop "+ count +", Phase 2 start : Do object field assignment, putfield \n");
      for(int i=count*_elem_num; i< (count+1)*_elem_num; i++ ){
        array[i].field_1 = new internalItem(i);
        array[i].field_2 = new internalItem(2*i);
        array[i].field_3 = new internalItem(3*i);
     //   array[i].field_4 = new internalItem(4*i);
      }
   
      // 3) reference some element objects, to prevent element-move-in oepration
       System.out.println("Loop "+ count +", Phase 3 start : Reference object array's 0-256 elements \n");
      for(int i=0; i< disturb_array.length; i++){
        // only disturb the first elements-move-in
        disturb_array[i] = array[i];
      }


      // 4) trigger minorGC/fullGC
    /*
      HashSet<objItem> set = new HashSet<objItem>();
      System.out.println("\n Phase 4 start : New a HashSet to trigger minorGC & fullGC.");

      for (int i=0; i<2*_elem_num; ++i) {
        set.add(new objItem(i,i*3));
      }
*/
      System.out.println("Phase4: Trigger full-gc");
/*
      System.out.println("Before Trigger gc, check the contents of array.");
      for(int i=0; i<array.length;i++){
        if( i%(array.length/8)==0){
          if(array[i] == null)
           System.out.println("array["+i+"] is null.");
          else
           System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val + ", internalItem field_3 id:" + array[i].field_3.id);
        } //check object at 1024*1024 granulirity
      }
*/
      System.out.println("Execute the GC.");
      System.gc();

      //check the content of the array

      System.out.println(" Final Phase start : Check the contents of array.");
      for(int i=0; i<array.length;i++){
        if( i%(array.length/8)==0){
          if(array[i] == null)
			     System.out.println("array["+i+"] is null.");
		      else
			     System.out.println("array["+i+"] -> key:" + array[i].key + ", val:" +array[i].val + ", internalItem field_3 id:" + array[i].field_3.id);
		    } //check object at 1024*1024 granulirity
      }

      for(int i=0; i<disturb_array.length; i++){
        if( i%(disturb_array.length/8)==0 ){
            if(disturb_array[i] == null)
              System.out.println("disturb_array["+i+"] is null.");
            else
              System.out.println("disturb_array["+i+"] -> key:" + disturb_array[i].key + ", val:" +disturb_array[i].val + ", internalItem field_3 id:" + disturb_array[i].field_3.id);
        }
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
//  internalItem field_4;


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






