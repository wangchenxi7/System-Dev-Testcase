import java.util.HashSet;
import java.util.ArrayList;

/*
* Performance Tuning Part
* Test path:
* a. Single gcThread 
* b. elements move-in efficiency 
*
* Big data array.
* 2 levels.
* Element is just Class objItem.
* 
*/


public class SparkCase40 {

  public static final int _elem_num		  = 16*1024*1024;        // length 8M  
  public static final int _small_elem_num = _elem_num/64;		// length 128K 
  public static objItem[][] static_array  = new objItem[20][];  // This obj array will be promoted to object_space during minorGC.

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
    do{

      System.out.println();
      System.out.println("Phase 1 start: build anewarray [objItem ");
      objItem t = new objItem(66,66*2);
      objItem[] array = new objItem[_elem_num];
      for (int i=0; i<array.length; ++i) {
        int factor = count+1;
        array[i] = new objItem( i*factor , i*factor*2 );
      }
      

      System.out.println();
      System.out.println("Phase 2 start: trigger a minorGC/FullGC in this loop.");                   //[!]trigger a minorGC in this loop.
      System.gc();

      System.out.println();
      System.out.println("Phase 3: change previous backbone to non-backbone");
      //record this local array to global static array
      static_array[count] = array;
      if(count > 0)
        static_array[count-1][0] = t;                         //make the previous array non-backbone

      System.out.println( "end of loop :" + count + " ,arrayList size :" + arrayList.size());
      System.out.println("");
    }while(++count < loop_num);

    //check the content
    for(int i=0;i < loop_num; i++){
      for(int j=0; j< static_array[i].length; j+=1024*1024)
        System.out.println( "static_array[" + i +"][" + j+" ], key:" + static_array[i][j].key + " val:" + static_array[i][j].val);
    }

    //trigger a minorGC/fullGC at last to check the result.
    System.gc();

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

