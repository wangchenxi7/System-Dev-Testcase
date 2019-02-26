import sun.misc.Unsafe;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class JVMonNVM01{
 
	public static int arraySize 		= 32*1024*1024;  // number 
  public static int objArraySize 	= 6*1024*1024;
	public static int itemSize 			= 2*1024*1024;
	public static int iteration		 	= 3;

  public static void main(String[] args){
	
	int i,j,k;

	// sun.misc.Unsafe unsafe;
	// try {
	//   Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
	//   unsafeField.setAccessible(true);
	//   unsafe = (sun.misc.Unsafe) unsafeField.get(null);
	// } catch (Throwable cause) {
	//   unsafe = null;
	// }

	//survive fullGC
	//double[][]  surviveGC = new double[iteration][];
	objItem[][]		surviveGC = new objItem[iteration][];

	for (j=0;j<iteration;j++){
	  int[] tmpA = new int[arraySize];
	  
	//  unsafe.enter_rdd_region();

	  //objItem tmp = new objItem(j,j*100);
//	  double[] tmpB = new double[arraySize];
	  objItem[] tmpC = new objItem[objArraySize]; 
	  

	  //init
	  for(i=0;i<arraySize; i++){
			tmpA[i]=i+4;
//		tmpB[i]=tmpA[i]+1;
	  }

	  for(i=0; i<objArraySize; i++){
//		if(i%512==0){
//		  System.out.println("assignment tmpC["+i+"]");
		  //tmp.obj = new internelObj(i);
		  tmpC[i] = new objItem(i,i*2);
		  tmpC[i].obj = new internelObj(i);
//		}
	  }

//	  unsafe.leave_rdd_region();

	  //calculate
	  double sum=0;
	  for(i=0; i<objArraySize; i+=512 ){
//		sum+=tmpA[i];
//		sum-=tmpB[i];
			sum+=tmpC[i].val;
	  }


	  // Add new object array into global list
	  surviveGC[j] = tmpC;

	  System.gc();
	  System.out.println("The sum is :"+ sum);
	} //for:j

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
