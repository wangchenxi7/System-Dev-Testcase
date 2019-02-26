
import java.util.Random;

import SparkCase34Base.*;

public final class SparkCase34 {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		int LENGTH = 16*1024*1024;

		/// initialise an RDD object with data
		RDD rdd = makeRDD( LENGTH * 2 );
		
		/// construct some function that will filter objects
		Filter filter = new EvenFilter();
		
		/// construct some function that will transform objects
		Expr expr = new AddConstant( 1 );
		
		/// perform some evaluation
		MapReduce mapReduce = new MapReduce( rdd, expr, filter );
		
		/// evaluate the map reduce
		RDD result = mapReduce.execute();
		
		assert( result.size() == LENGTH );
		
		int count = 0;
		for( RDDIterForward iter = result.begin(); iter.hasNext(); count++ ) {
			Datum datum = iter.next();
			
			int expected = (count + 1)*2;
			
			assert( datum.value == expected );
		}
		
		assert( count == LENGTH );


		//debug, check the result of RDD data
		for(int i=0; i< LENGTH; i+=LENGTH/10){
			System.out.println( "RDD [" + i + "] : " + result.get_elem(i).value);
		}

	}

	static RDD makeRDD( int total ) {
		Random rand = new Random();
		
		RDD rdd = new RDD( total );
		
		for( int i = 0; i<total; ++i ) {
			Datum datum = new Datum( i );
			
			rdd.add( datum );
		}
		
		return rdd;
	}
}

class AddConstant extends Expr {

	AddConstant( int value ) {
		this.value = value;
	}
	
	@Override
	public Datum map(Datum datum) {
		return new Datum( datum.value + this.value );
	}

	private int value;
}

class EvenFilter extends Filter {
	
	public boolean keep( Datum datum ) {
		return datum.value % 2 == 0;
	}
}
