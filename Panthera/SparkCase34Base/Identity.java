package SparkCase34Base;

public class Identity extends Expr {

	@Override
	public Datum map( Datum datum ) {
		return new Datum( datum );				//This computation just builds a datum with same value.
	}

}
