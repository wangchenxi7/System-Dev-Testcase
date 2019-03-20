import java.util.HashSet;
import java.util.ArrayList;

/*
* Test path:
* Trigger minorGC by running out region space.
* But can't free enough space, so allocate recognized arrays into nomarl path.  
*
* Big data array.
* 2 levels.
* Element is just Class Object.
* 
*/


public class SparkCase10 {

  public static  Object[] static_array;
  public static final int _elem_num       = 8*1024*1024;            // 8M elements   
  public static final int _small_elem_num = _elem_num/(8*1024*8);   // 128  

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
    objItem[][] record_array = new objItem[_elem_num*2][];                       // Can store 8M objItem object array
    int index;

    while (count++ < loop_num) {
     
      if(count%2==0)    //This can free lots objItem array
        index = 0;
      else 
        index =1;

      System.out.println("Phase 1 new lots objItem array, to run out the region space.");
      for (int i=index*_small_elem_num; i< (index+1)*_small_elem_num; ++i) {
        objItem[] tmp_array = new objItem[_elem_num/8];       // a big objItem array
        record_array[i] = tmp_array;
        
        if(count % 64 ==0)
          sleep(1000);
      }


      System.out.println("Phase 2 assign some array elements to each objItem array");
      for (int i= index*_small_elem_num; i< (index + 1)*_small_elem_num; ++i) {
        record_array[i][0] = new objItem(i, i*2);
      }


      System.out.println("");
      System.out.println( "end of loop :" + count );
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
//  double[] content = new double[testGC.itemSize]; //2MB
  internelObj obj;

  public objItem(int key, int val){
  this.key =key;
  this.val = val;

//  System.out.println("build objItem instance, key:"+key);
//  int i;
//  for(i=0;i<testGC.itemSize;i++)
//    content[i]=i*val;
  }

}



class internelObj{
  int val;

  public internelObj(int val){
  this.val = val;
  }
}




