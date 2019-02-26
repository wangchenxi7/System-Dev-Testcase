package SparkCase34Base;

public class Compose extends Expr {

	public Compose( Expr f, Expr g ) {
		this.f = f;
		this.g = g;
	}
	
	public Datum map( Datum datum ) {
		return f.map(g.map(datum)); 
	}
	
	private Expr f;
	private Expr g;
}
