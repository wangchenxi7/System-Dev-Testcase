import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* The "anewarray" may be optimized by C1/Opto Compiler
* Then it will not invoke the CollectedHeap::obj_allocate(KlassHandle klass, int size, TRAPS)
* Which can escape our object array allocation recognization. 
*
*
* Big data array.
* 2 levels.
* Element is just Class objItem.
* 
*/


public class SparkCase21 {

  public static final int _elem_num = 1*1024*1024;          // length 1M  
  public static final int _small_elem_num = _elem_num/64;   // length 8K 
  public static objItem[][] static_array = new objItem[20][];   // This obj array will be promoted to object_space during minorGC.

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
     
     System.out.println("Phase 1 start:");
      for (int i=0; i< _small_elem_num; ++i) {
        arrayList.add(new objItem(i,i));        //when ArrayList scales, it build a new one & copy the contents.
      }

      System.out.println();
      System.out.println("Phase 2 start: build anewarray [objItem ");
      objItem t = new objItem(66,66*2);
      objItem[] array = new objItem[_elem_num];
      for (int i=0; i<array.length; ++i) {
        int factor = count+1;
        array[i] = new objItem( i*factor , i*factor*2 );
      }
      objItem[] tmpArray = new objItem[_small_elem_num];
      tmpArray = new objItem[_small_elem_num];

      sleep(2000);

      System.out.println();
      System.out.println("Phase 3 start: build anewarray [objItem ");
      objItem t2 = new objItem(77,77*2);
      objItem[] array2 = new objItem[_elem_num];
      for (int i=0; i<array2.length; ++i) {
        int factor = count+5;
        array2[i] = new objItem( i*factor , i*factor*2 );
      }
      
      tmpArray = new objItem[_small_elem_num];
      tmpArray = new objItem[_small_elem_num];

      sleep(2000);

      System.out.println();
      System.out.println("Phase 4: record produced obejct array");
      //record this local array to global static array
      static_array[count] = array;
      static_array[count+5] = array2;
    

      System.out.println( "************* end of loop :" + count + " ,arrayList size :" + arrayList.size() +"***************");
      System.out.println("\n\n");

    }while(++count < loop_num);

    //check the content
    for(int i=0;i < loop_num; i++){
      for(int j=0; j< static_array[i].length; j+=256*1024)
        System.out.println( "static_array[" + i +"][" + j+" ], key:" + static_array[i][j].key + " val:" + static_array[i][j].val);
    }


    System.out.println("");
    System.out.println("****************PASSED****************");

  }





  public static void main(String args[]) {

    region_loop(4);
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

