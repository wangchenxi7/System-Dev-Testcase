/**
 * Basic Java program for GC performance test.
 * 
 */
public class Simple {

    public static final int _elem_num       = 12*1024*1024;      // 4M elements
    public static final int _small_elem_num = _elem_num/128;    // 32K elements
  
    public static void sleep(long ms) {
      try {
        System.out.println("Go sleeping for " + ms/1000 + "!");
        Thread.sleep(ms);ß
        System.out.println("Wake up and done!");
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

  
    public static void region_loop(int loop_num, objItem[] array) {
  
      int count = 0;
  
      do{ 
        // 1) Check object array assignment : aastore
        System.out.println("Loop "+ count +", Phase 1 start : Do object array element assignment, aastore \n");
  
        // Only keep the latest 2 arrays 
        if( (count+1)*_small_elem_num  >= array.length ){
          // 1) move array[1] to array[0] 
          System.arraycopy(array,array.length/2, array,0, array.length/2);
        }
        
        // 2) Assign newly build array to array[1]
        for(int i= 0; i< _small_elem_num; i++ ){
          array[i+_small_elem_num] = new objItem(i*(count+1), 2*i*(count+1));
        }
        // sleep 2s
       // sleep(2000);
  
        // 2) do object field assignment : putfield
        System.out.println("Loop "+ count +", Phase 2 start : Do object field assignment, putfield \n");
        for(int i=_small_elem_num; i< array.length; i++ ){
          array[i].field_1 = new internalItem((count+1)*i);
          array[i].field_2 = new internalItem((count+1)*2*i);
          array[i].field_3 = new internalItem((count+1)*3*i);
          array[i].field_4 = new internalItem((count+1)*4*i);
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
  
      //the array to be checked.
      objItem[] array = new objItem[_small_elem_num*2];
      
      region_loop(16, array);
    }
  }
  
  // obj header +
  // offload: size 4*2 + 8*4 = 40 bytes
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
  
  // obj header
  // offload :size 4 bytes
  class internalItem{
    int id;
  
    public internalItem(int id){
      this.id = id;
    }
  }
  
  
