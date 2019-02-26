package SparkCase34Base;

import java.util.Random;



public class Generate {

	private Random r = new Random();
	
	public Generate( )
	{
	}
	
	Datum generate()
	{
		return new Datum( r );
	}
}
