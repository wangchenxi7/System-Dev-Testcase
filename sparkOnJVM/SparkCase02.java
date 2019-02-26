import sun.misc.Unsafe;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

/*
* Test path:
* a. Can allocate object array into Region Space directly.
*	b. MinorGC can gather elements into Region Space according to StarTask->_dram flag.
* c. MajorGC : destination sequence : Region Space -> Object Space -> Young Gen 
*
* Big data array.
* 2 levels.
* Element is just Class Object.
* 
*/



public class SparkCase02{
 
	public static int arraySize    =	128;  		  // number 
  public static int objArraySize =	2 * 1028;	  // 2K elements
 //	public static int itemSize     = 	2 * 1024; // 2K elements 
 	public static int iteration    = 	8;

  public static void main(String[] args){
	
		int i,j,k;

		sun.misc.Unsafe unsafe;
		try {
	  	Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
	  	unsafeField.setAccessible(true);
	  	unsafe = (sun.misc.Unsafe) unsafeField.get(null);
		} catch (Throwable cause) {
	  	unsafe = null;
		}

		//survive fullGC
		unsafe.enter_rdd_region();
	//	objItem[][] surviveFullGC = new objItem[iteration][]; 
		objItem[][] surviveFullGC = new objItem[1024*1024][]; 
		objItem[][] surviveFullGC_2 = new objItem[1024*1024][]; 
		objItem[][] surviveFullGC_3 = new objItem[1024*1024][]; 
		unsafe.leave_rdd_region();

		//debug
		System.gc();

		for (j=0; j<iteration; j++){
	  
	  	System.out.println("Iteration " + j + " Start: \n");
		
	  	int factor = j+1;

	  	if(j%2 == 1){
	  		//used for check results
	  		System.out.println("\n Only trigger minorGC and fullGC, do nothing.\n");
	  		System.gc();

	  		// check the result of last iteration
	  		System.out.println("Check the result  of last iteration.");
	  		for(i=0; i<objArraySize; i+=512 ){
					System.out.println("surviveFullGC[" + (j-1) + "]["+ i +"] :" + "key :" +surviveFullGC[j-1][i].key  + ", val :" + surviveFullGC[j-1][i].val);
					System.out.println("");
	  		}
	  		continue;
	  	}


			System.out.println("Phase 1: allocate primitive array and object array.");
	  	int[] tmpA = new int[arraySize];							//primitive array

	  	objItem[] tmpC = new objItem[objArraySize];   // object array

	  	System.out.println("Phase 2: Init primitive array && assign elements to object array.");
	  	//init
	  	for(i=0;i<arraySize; i++){
				tmpA[i]=i+4;
	  	}

	  	for(i=0; i<objArraySize; i++){
		 		tmpC[i] = new objItem(i*factor, i*2*factor);   // objItem instance, allocate into eden space.
	  	}

	//  unsafe.leave_rdd_region();

	  	System.out.println("Phase 3: trigger MinorGC & FullGC via System.gc()\n");
	  	System.gc();

	  	//calculate
	  	double sum=0;
	  	for(i=0; i<objArraySize; i+=512 ){
				System.out.println("tmpC[" + i + "] :" + "key :" + tmpC[i].key + ", val :" + tmpC[i].val);
				sum+=tmpC[i].val;
	  	}

	  	//make the object array survive GC
			surviveFullGC[j] = tmpC;
			surviveFullGC_2[j] = tmpC;
			surviveFullGC_3[j] = tmpC;
			
	  	
	  	System.out.println("End of Iteration " + j + ":  The sum is :"+ sum);
	  	System.out.println("\n");
		} //for:j

		System.out.println("Check all the results:");
		//check all the results
		for(j=0; j< iteration; j+=2){  // only iteration_num%2 != 0 has object array assignment
			for(i=0; i<objArraySize; i+=512 ){
				System.out.println("surviveFullGC[" + j + "]["+i+"]:" + "key :" + surviveFullGC[j][i].key + ", val :" + surviveFullGC[j][i].val);
			}
		}

		System.out.println("*********ALL PASSED*******");

  }// main
}


class objItem{
  int  key;
  int  val;
//  double[] content = new double[testGC.itemSize]; //2MB
  internelObj obj;

  public objItem(int key, int val){
	this.key =key;
	this.val = val;

//	System.out.println("build objItem instance, key:"+key);
	int i;
//	for(i=0;i<testGC.itemSize;i++)
//	  content[i]=i*val;
  }

}

class internelObj{
  int val;

  public internelObj(int val){
	this.val = val;
  }
}
